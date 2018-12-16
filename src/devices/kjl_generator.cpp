#include "kjl_generator.h"

KJLGenerator::KJLGenerator(std::unique_ptr<BaseConnector> &&connector) : RFGenerator(std::move(connector)) {
    if (m_connector) {
        QObject::connect(m_connector.get(), &BaseConnector::data_received, this, &KJLGenerator::handle_data_received);
    }
}

void KJLGenerator::set_connector(std::unique_ptr<BaseConnector> &&connector) {
    Device::set_connector(std::move(connector));

    if (m_connector) {
        QObject::connect(m_connector.get(), &BaseConnector::data_received, this, &KJLGenerator::handle_data_received);
    }
}

void KJLGenerator::init(std::shared_ptr<config::Segment> settings) {
    if (!m_connector) {
        logging::get_log("main")->warn(
            "KJLGenerator: init called but no connector was set up. Call set_connector with a valid connector first.");
        return;
    }

    m_connector->init(settings);
    if (!m_connector->connect()) {
        logging::get_log("main")->error(
            "KJLGenerator: an error occured while trying to connect to the generator. Connector info:\n  {0}", m_connector->info());
    } else {
        logging::get_log("main")->debug("KJLGenerator: connected. Connector info:\n  {0}", m_connector->info());
    }
}

void KJLGenerator::update() {
    // See if we have captured all parameters
    if (m_generator_parameters.external_feedback != -1 && m_generator_parameters.forward_power != -1
        && m_generator_parameters.reflected_power != -1 && m_generator_parameters.setpoint != -1
        && m_generator_parameters.load_cap_position != -1 && m_generator_parameters.tune_cap_position != -1) {
        std::scoped_lock<std::mutex> lock(m_parameter_mutex);

        emit update_parameters(m_id);

        // Reset all parameters to indicate we still need to save them
        m_generator_parameters = {-1, -1, -1, -1, -1, -1};
    }
}

void KJLGenerator::output_on() { send('G', '\r'); }

void KJLGenerator::output_off() { send('S', '\r'); }

void KJLGenerator::set_target_power(int power) {
    if (power < 0) {
        logging::get_log("main")->warn("KJLGenerator: set_target_power called with negative power value of {0}", power);
        return;
    }
    // TODO: test for maximum power

    send(QByteArray::fromStdString(fmt::format("{0}.0 W\r", power)));
}

void KJLGenerator::set_load_capacitor_position(int position) {
    if (position < 0 || position > 100) {
        logging::get_log("main")->warn(
            "KJLGenerator: set_load_capacitor_position called with position outside the range [0, 100], got {0}", position);
        return;
    }

    send(QByteArray::fromStdString(fmt::format("{0} MPL\r", position)));
}

void KJLGenerator::set_tune_capacitor_position(int position) {
    if (position < 0 || position > 100) {
        logging::get_log("main")->warn(
            "KJLGenerator: set_tune_capacitor_position called with position outside the range [0, 100], got {0}", position);
        return;
    }

    send(QByteArray::fromStdString(fmt::format("{0} MPT\r", position)));
}

void KJLGenerator::query_capacitor_positions() {
    send(QByteArray::fromStdString("LPS\r"));  // load capacitor
    send(QByteArray::fromStdString("TPS\r"));  // tune capacitor
}

void KJLGenerator::query_external_feedback() { send('0', '?', '\r'); }

void KJLGenerator::query_forward_power() { send('W', '?', '\r'); }

void KJLGenerator::query_reflected_power() { send('R', '?', '\r'); }

void KJLGenerator::query_status() { send('Q', '\r'); }

void KJLGenerator::handle_data_received(const QByteArray &data) {
    // A buffer for incoming messages so we don't keep the mutex locked for actual handling of the packets
    std::vector<QByteArray> replies;

    // TODO: keep track of sent commands to identify lost packets

    {
        std::scoped_lock<std::mutex> lock(m_data_mutex);

        m_in_buffer.append(data);

        // We are in echo mode, so the reply is "<command><cr><answer><cr>", where <answer> may be "N" for NACK
        while (m_in_buffer.count('\r') >= 2) {
            int first_index = m_in_buffer.indexOf('\r');
            int end_index = m_in_buffer.indexOf('\r', first_index + 1);

            replies.push_back(m_in_buffer.left(end_index + 1));
            m_in_buffer = m_in_buffer.right(m_in_buffer.length() - end_index - 1);
        }
    }  // scoped_lock

    // Process received replies
    for (const auto &r : replies) {
        handle_reply(r);
    }
}

void KJLGenerator::handle_reply(const QByteArray &data) {
    int first_index = data.indexOf('\r');
    if (first_index == -1) {
        logging::get_log("main")->warn("KJLGenerator: no <cr> in reply, got '{0}' (hex)", data.toHex().toStdString());
        return;
    }

    int second_index = data.indexOf('\r', first_index + 1);
    if (second_index == -1) {
        logging::get_log("main")->warn("KJLGenerator: missing second <cr> in reply, got '{0}' (hex)", data.toHex().toStdString());
        return;
    }

    // There should be at least a second <cr> if the command was accepted
    if (data.length() < first_index + 1) {
        logging::get_log("main")->warn("KJLGenerator: missing data in reply, got '{0}' (hex)", data.toHex().toStdString());
        return;
    }

    if (auto ack = data[first_index + 1]; ack == 'N') {
        // Command not accepted
        logging::get_log("main")->warn("KJLGenerator: command '{0}' was rejected by the generator", data.left(first_index + 1));
        return;
    }

    char cmd = data[0];
    switch (cmd) {
    case '0':  // 0x30, external feedback (dc bias voltage)
    {
        if (data[1] != '?') {
            logging::get_log("main")->warn("KJLGenerator: received unknown reply command, got '{0}' (hex)",
                                           data.toHex().toStdString());
            return;
        }

        std::scoped_lock<std::mutex> lock(m_parameter_mutex);
        m_generator_parameters.external_feedback = data.mid(3, second_index - 3).toInt();

        logging::get_log("main")->debug("KJLGenerator: external feedback returned {0}", m_generator_parameters.external_feedback);
        break;
    }
    case 'L':  // load capacitor position
    {
        std::scoped_lock<std::mutex> lock(m_parameter_mutex);
        m_generator_parameters.load_cap_position = data.mid(2, second_index - first_index - 1).toInt();

        logging::get_log("main")->debug("KJLGenerator: load capacitor position power returned {0}",
                                        m_generator_parameters.load_cap_position);
        break;
    }
    case 'T':  // tune capacitor position
    {
        std::scoped_lock<std::mutex> lock(m_parameter_mutex);
        m_generator_parameters.load_cap_position = data.mid(2, second_index - first_index - 1).toInt();

        logging::get_log("main")->debug("KJLGenerator: tune capacitor position power returned {0}",
                                        m_generator_parameters.load_cap_position);
        break;
    }
    case 'Q':  // 0x51
    {
        // Q returns "Q<cr>XXXXXXX aaaa bbbb ccc dddd", where
        //     a is setpoint in Watts
        //     b is forward power in Watts
        //     c is reflected power in Watts
        //     d is maximum power in Watts
        //     X holds additional information
        auto tokens = *util::split(data.mid(2, second_index - first_index - 1).toStdString(), ' ');
        if (tokens.empty()) {
            logging::get_log("main")->warn("KJLGenerator: received corrupted reply to 'Q' (QueryStatus) command, got '{0}' (hex)",
                                           data.toHex().toStdString());
            return;
        }

        std::scoped_lock<std::mutex> lock(m_parameter_mutex);

        m_generator_parameters.setpoint = *util::to_type<int>(tokens[1]);
        m_generator_parameters.forward_power = *util::to_type<int>(tokens[2]);
        m_generator_parameters.reflected_power = *util::to_type<int>(tokens[3]);

        logging::get_log("main")->debug(
            "KJLGenerator: reflected power returned {0} (setpoint), {1} (forward power}, {2} (reflected power)",
            m_generator_parameters.setpoint, m_generator_parameters.forward_power, m_generator_parameters.reflected_power);
        break;
    }
    case 'R':  // 0x52, reflected power
    {
        std::scoped_lock<std::mutex> lock(m_parameter_mutex);
        m_generator_parameters.reflected_power = data.mid(3, 4).toInt();

        logging::get_log("main")->debug("KJLGenerator: reflected power returned {0}", m_generator_parameters.reflected_power);
        break;
    }
    case 'W':  // 0x57, forward power
    {
        std::scoped_lock<std::mutex> lock(m_parameter_mutex);
        m_generator_parameters.forward_power = data.mid(3, 4).toInt();

        logging::get_log("main")->debug("KJLGenerator: forward power returned {0}", m_generator_parameters.forward_power);
        break;
    }
    default:
        logging::get_log("main")->warn("KJLGenerator: received unknown reply command, got {0} (hex)", data.toHex().toStdString());
    }
}