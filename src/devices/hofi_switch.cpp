#include "hofi_switch.h"

#include "logging/logging.h"

HofiSwitch::HofiSwitch(std::unique_ptr<BaseConnector> &&connector) : RFSwitch(std::move(connector)) {
    if (m_connector) {
        QObject::connect(get_connector(), &BaseConnector::data_received, this, &HofiSwitch::handle_data_received);
    }
}

void HofiSwitch::set_connector(std::unique_ptr<BaseConnector> &&connector) {
    Device::set_connector(std::move(connector));

    if (m_connector) {
        QObject::connect(get_connector(), &BaseConnector::data_received, this, &HofiSwitch::handle_data_received);
    }
}

void HofiSwitch::init(std::shared_ptr<config::Segment> settings) {
    if (!m_connector) {
        logging::get_log("main")->warn(
            "HofiSwitch: init called but no connector was set up. Call set_connector with a valid connector first.");
        return;
    }

    m_connector->init(settings);
    if (!m_connector->connect()) {
        logging::get_log("main")->error(
            "HofiSwitch: an error occured while trying to connect to the switch. Connector info:\n{0}", m_connector->info());
    } else {
        logging::get_log("main")->debug("HofiSwitch: connected. Connector info:\n{0}", m_connector->info());
        
        // query current status
        m_connector->write(QByteArray("HOFISTATU"));
    }
}

void HofiSwitch::set_port(int8_t port) {
    if (port < 1 || port > 5) {
        logging::get_log("main")->error("HofiSwitch: port outside range [1, 5], got {0}", port);
        return;
    }

    m_connector->write(QByteArray::fromStdString(fmt::format("HOFIPORT{0}", port)));
    // query back the status
    m_connector->write(QByteArray("HOFISTATU"));
}

int8_t HofiSwitch::get_port() { return m_current_port; }

void HofiSwitch::handle_data_received(const QByteArray &data) {
    std::scoped_lock<std::mutex> lock(m_data_mutex);

    logging::get_log("main")->debug("HofiSwitch: received '{0}'", data.toHex().toStdString());

    m_in_buffer.append(data);

    // No manual currently available, so we just check the last byte received, disregarding possible previous messages
    // and errors

    if (m_in_buffer.length() > 3) {
        const int8_t port = m_in_buffer.back();
        if (port > 0 && port < 6 || port == 0xa) {
            m_current_port = port;

            emit port_changed(port);
        } else {
            logging::get_log("main")->warn("HofiSwitch: unrecognized reply from switch, got '{0}'", m_in_buffer.toStdString());
        }

        m_in_buffer.clear();
    }
}