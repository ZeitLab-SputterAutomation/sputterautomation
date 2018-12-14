#include "cesar_generator.h"

#include "app_config.h"

CesarGenerator::CesarGenerator(std::unique_ptr<BaseConnector> &&connector) : RFGenerator(std::move(connector)) {
    if (m_connector) {
        QObject::connect(m_connector.get(), &BaseConnector::data_received, this, &CesarGenerator::handle_data_received);
    }
}

void CesarGenerator::set_connector(std::unique_ptr<BaseConnector> &&connector) {
    Device::set_connector(std::move(connector));

    if (m_connector) {
        QObject::connect(m_connector.get(), &BaseConnector::data_received, this, &CesarGenerator::handle_data_received);
    }
}

void CesarGenerator::init(std::shared_ptr<config::Segment> settings) {
    if (!m_connector) {
        logging::get_log("main")->warn(
            "CesarGenerator: init called but no connector was set up. Call set_connector with a valid connector first.");
        return;
    }

    m_connector->init(settings);
    if (!m_connector->connect()) {
        logging::get_log("main")->error(
            "CesarGenerator: an error occured while trying to connect to the generator. Connector info:\n{0}",
            m_connector->info());
    } else {
        logging::get_log("main")->debug("CesarGenerator: connected. Connector info:\n{0}", m_connector->info());
    }
}

void CesarGenerator::update() {
    if (m_generator_parameters.external_feedback != -1 && m_generator_parameters.forward_power != -1
        && m_generator_parameters.reflected_power != -1 && m_generator_parameters.setpoint != -1
        && m_generator_parameters.load_cap_position != -1 && m_generator_parameters.tune_cap_position != -1) {
        std::scoped_lock<std::mutex> lock(m_parameter_mutex);

        emit update_parameters();

        // Reset all parameters to indicate we still need to save them
        m_generator_parameters = {-1, -1, -1, -1, -1, -1};
    }
}

void CesarGenerator::set_control_mode(ControlMode mode) {
    uint8_t mode_id = 0;
    if (mode == ControlMode::Local) {
        mode_id = 6;
    } else if (mode == ControlMode::Remote) {
        mode_id = 2;
    } else {
        logging::get_log("main")->debug("CesarGenerator: ignoring unsupported control mode {0:#b}", static_cast<uint8_t>(mode));
        return;
    }

    queue_command(Commands::SelectActiveControlMode, mode_id);
}

void CesarGenerator::output_on() { 
    queue_command(Commands::OutputOn); 
}

void CesarGenerator::output_off() { 
    queue_command(Commands::OutputOff); 
}

void CesarGenerator::set_target_power(int power) {
    if (power < 0) {
        logging::get_log("main")->warn("CesarGenerator: set_target_power called with negative power value of {0}", power);
        return;
    }
    // TODO: test for maximum power

    queue_command(Commands::SetPowerSetPoint, power & 0xFF, (power >> 8) & 0xFF);
}

void CesarGenerator::set_load_capacitor_position(int position) {
    // Note: Values taken from cesar hardware manual, page 4-70
    if (position < 40 || position > 960) {
        logging::get_log("main")->warn(
            "KJLGenerator: set_load_capacitor_position called with position outside the range [40, 960], got {0}", position);
        return;
    }

    queue_command(Commands::MoveLoadCapPosition, position & 0xFF, (position >> 8) & 0xFF);
}

void CesarGenerator::set_tune_capacitor_position(int position) {
    // Note: Values taken from cesar hardware manual, page 4-71
    if (position < 40 || position > 960) {
        logging::get_log("main")->warn(
            "KJLGenerator: set_tune_capacitor_position called with position outside the range [40, 960], got {0}", position);
        return;
    }

    queue_command(Commands::MoveTuneCapPosition, position & 0xFF, (position >> 8) & 0xFF);
}

void CesarGenerator::set_matchnetwork_mode(MatchnetworkMode mode) {
    uint8_t mode_id = 0;
    if (mode == MatchnetworkMode::Automatic) {
        mode_id = 1;
    } else if (mode == MatchnetworkMode::Manual) {
        mode_id = 0;
    } else {
        logging::get_log("main")->debug("CesarGenerator: ignoring unknown match control mode id {0}", static_cast<int>(mode));
        return;
    }

    queue_command(Commands::SetMatchNetworkControl, mode_id);
}

void CesarGenerator::query_capacitor_positions() { 
    queue_command(Commands::ReportCapacitorPositions);
}

void CesarGenerator::query_external_feedback() { 
    queue_command(Commands::ReportExternalFeedback); 
}

void CesarGenerator::query_forward_power() { 
    queue_command(Commands::ReportForwardPower); 
}

void CesarGenerator::query_reflected_power() { 
    queue_command(Commands::ReportReflectedPower); 
}

void CesarGenerator::queue_command(const QByteArray &command) {
    if (!m_connector) {
        logging::get_log("main")->warn("CesarGenerator: queue_command was called but no connector was set up, command was '{0}'", command.toHex().toStdString());
        return;
    }

    std::scoped_lock<std::mutex> lock(m_command_mutex);

    if (m_command_queue.empty()) {
        m_connector->write(command);
    }

    m_command_queue.push_back(command);
}

bool CesarGenerator::write_command() {
    if (!m_command_queue.empty()) {
        m_connector->write(m_command_queue.front());
        return true;
    }
    return false;
}

void CesarGenerator::handle_data_received(const QByteArray &data) {
    // A buffer for incoming messages so we don't keep the mutex locked for actual handling of the packets
    std::vector<Packet> packets;

    {
        std::scoped_lock<std::mutex> data_lock(m_data_mutex);

        // Since we can only ever send one command at once we have to keep track of when we send one
        bool command_sent = false;

        m_in_buffer.append(data);

        // Process incoming and buffered data
        int i = 0;
        for (; i < m_in_buffer.length(); i++) {
            const uint8_t header = m_in_buffer[i];

            if (header == ACK) {
                // ACK: command successfully sent, delete it from the queue
                std::scoped_lock<std::mutex> command_lock(m_command_mutex);
                if (!m_command_queue.empty()) {
                    m_command_queue.pop_front();
                }
                m_send_retries = 0;

                // If no command was sent yet send the next one (if available)
                if (!command_sent) {
                    command_sent = write_command();
                }

                // There may be more data (for example an answer to a request) so keep iterating
                continue;
            } else if (header == NACK) {
                // NACK: resend the packet or throw it away if we tried too many times already
                if (m_send_retries >= MAX_SEND_RETRIES) {
                    std::scoped_lock<std::mutex> command_lock(m_command_mutex);
                    if (!m_command_queue.empty()) {
                        logging::get_log("main")->warn("CesarGenerator: sending of command '{0}' unsuccessul after {1} attempts",
                                                       m_command_queue.front().toStdString(), MAX_SEND_RETRIES);

                        m_command_queue.pop_front();
                    } else {
                        logging::get_log("main")->warn(
                            "CesarGenerator: sending of command unsuccessful after {0} attempts, command queue is empty",
                            MAX_SEND_RETRIES);
                    }
                    m_send_retries = 0;
                }

                m_send_retries++;
                if (!command_sent) {
                    command_sent = write_command();
                } else {
                    // NACK received after ACK, or multiple NACKs received. This shouldn't happen.
                    logging::get_log("main")->warn("CesarGenerator: multiple NACKs or ACK + NACK received");
                }

                m_in_buffer.clear();

                // Stop iterating since we cleared the buffer
                break;
            }

            // Length of data is the last three bits of the header byte, or an optional byte with offset 2 if the length
            // is greater than 6
            int datalength = header & 0b111;
            int offset = 2;
            if (datalength == 7) {
                if (i + 2 >= m_in_buffer.length()) {
                    break;
                }

                datalength = m_in_buffer[i + 2];
                offset = 3;
            }

            // Check if we have enough data buffered (+1 for the checksum byte)
            if (m_in_buffer.length() < i + offset + datalength + 1) {
                break;
            }

            // Compare the checksum
            char checksum = 0;
            for (int j = i; j < i + offset + datalength + 1; j++) {
                checksum ^= m_in_buffer[j];
            }

            if (checksum == 0) {
                m_connector->write(QByteArray(1, ACK));
                packets.push_back({header, m_in_buffer.mid(i + offset - 1, datalength + 1)});
            } else {
                m_connector->write(QByteArray(1, NACK));
                logging::get_log("main")->warn("CesarGenerator: received corrupted packet with content\n\t{0} (hex)",
                                               m_in_buffer.mid(i, datalength + 1).toHex().toStdString());
            }

            i += offset + datalength;
        }

        if (!m_in_buffer.isEmpty()) {
            // Clear processed data from the buffer
            if (i == m_in_buffer.length()) {
                m_in_buffer.clear();
            } else {
                m_in_buffer = m_in_buffer.mid(i);
            }
        }
    }  // scoped_lock

    // Process received packets
    for (const auto &p : packets) {
        handle_reply(p);
    }
}

void CesarGenerator::handle_reply(const Packet &packet) {
    switch (static_cast<Commands>(packet.command)) {
    // case Commands::ReportPowerSupplyType:
    //    break;
    // case Commands::ReportMatchNetworkMotorMovement:
    //    break;
    // case Commands::ReportReflectedPowerParameters:
    //    break;
    // case Commands::ReportRegulationMode:
    //    break;
    // case Commands::ReportActiveControlMode:
    //    break;
    // case Commands::ReportProcessStatus:
    //    break;
    // case Commands::ReportMatchNetworkControlMode:
    //    break;
    case Commands::ReportSetPointAndRegulationMode: {
        if (packet.data.length() < 2) {
            logging::get_log("main")->warn(
                "CesarGenerator: not enough data in the reply to ReportSetPointAndRegulationMode, got '{0}'",
                packet.data.toHex().toStdString());
            return;
        }

        std::scoped_lock<std::mutex> lock(m_parameter_mutex);
        m_generator_parameters.setpoint = (packet.data[1] << 8) | packet.data[0];

        logging::get_log("main")->debug("CesarGenerator: ReportSetPointAndRegulationMode returned {0}",
                                        m_generator_parameters.setpoint);
        break;
    }
    case Commands::ReportForwardPower: {
        if (packet.data.length() < 2) {
            logging::get_log("main")->warn("CesarGenerator: not enough data in the reply to ReportForwardPower, got '{0}'",
                                           packet.data.toHex().toStdString());
            return;
        }

        std::scoped_lock<std::mutex> lock(m_parameter_mutex);
        m_generator_parameters.forward_power = (packet.data[1] << 8) | packet.data[0];

        logging::get_log("main")->debug("CesarGenerator: ReportForwardPower returned {0}", m_generator_parameters.forward_power);
        break;
    }
    case Commands::ReportReflectedPower: {
        if (packet.data.length() < 2) {
            logging::get_log("main")->warn("CesarGenerator: not enough data in the reply to ReportReflectedPower, got '{0}'",
                                           packet.data.toHex().toStdString());
            return;
        }

        std::scoped_lock<std::mutex> lock(m_parameter_mutex);
        m_generator_parameters.reflected_power = (packet.data[1] << 8) | packet.data[0];

        logging::get_log("main")->debug("CesarGenerator: ReportReflectedPower returned {0}",
                                        m_generator_parameters.reflected_power);
        break;
    }
    // case Commands::ReportDeliveredPower:
    //    break;
    case Commands::ReportExternalFeedback: {
        if (packet.data.length() < 2) {
            logging::get_log("main")->warn("CesarGenerator: not enough data in the reply to ReportExternalFeedback, got '{0}'",
                                           packet.data.toHex().toStdString());
            return;
        }

        std::scoped_lock<std::mutex> lock(m_parameter_mutex);
        m_generator_parameters.external_feedback = (packet.data[1] << 8) | packet.data[0];

        logging::get_log("main")->debug("CesarGenerator: ReportExternalFeedback returned {0}",
                                        m_generator_parameters.external_feedback);
        break;
    }
    // case Commands::ReportForwardPowerLimit:
    //    break;
    // case Commands::ReportReflectedPowerLimit:
    //    break;
    case Commands::ReportCapacitorPositions: {
        if (packet.data.length() < 4) {
            logging::get_log("main")->warn("CesarGenerator: not enough data in the reply to ReportCapacitorPositions, got '{0}'",
                                           packet.data.toHex().toStdString());
            return;
        }

        std::scoped_lock<std::mutex> lock(m_parameter_mutex);
        m_generator_parameters.load_cap_position = ((packet.data[1] << 8) | packet.data[0]) / 10;
        m_generator_parameters.tune_cap_position = ((packet.data[3] << 8) | packet.data[2]) / 10;

        logging::get_log("main")->debug("CesarGenerator: ReportCapacitorPositions returned {0} (load cap) and {1} (tune cap)",
                                        m_generator_parameters.load_cap_position, m_generator_parameters.tune_cap_position);
        break;
    }
    // case Commands::ReportUnitRunTime:
    //    break;
    case Commands::ReportFaultStatusRegister: {
        if (packet.data.length() < 2) {
            logging::get_log("main")->warn("CesarGenerator: not enough data in the reply to ReportFaultStatusRegister, got '{0}'",
                                           packet.data.toHex().toStdString());
            return;
        }

        int faultstatus = (packet.data[1] << 8) | packet.data[0];

        logging::get_log("main")->debug("CesarGenerator: ReportFaultStatusRegister returned {0}", faultstatus);
        break;
    }

    case Commands::ErrorMatchingNetworkNotConnected: {
        // No matching network connected
        logging::get_log("main")->warn("CesarGenerator: no matching network connected");
        break;
    }

    default:
        logging::get_log("main")->warn("CesarGenerator: received unknown reply command {0}", packet.command);
    }
}
