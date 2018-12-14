#include "device.h"

Device::Device() noexcept : m_id{get_id()} {};

Device::Device(std::unique_ptr<BaseConnector> &&connector) : m_connector{std::move(connector)}, m_id{get_id()} {};

void Device::set_connector(std::unique_ptr<BaseConnector> &&connector) {
    if (m_connector != nullptr) {
        logging::get_log("main")->warn("Device: overwriting connector {0}", m_connector->info());
    }

    m_connector = std::move(connector);
}

bool Device::is_connected() {
    if (!m_connector) {
        logging::get_log("main")->error("Device: is_connected was called but not connector was set up");
        return false;
    }

    return m_connector->is_connected();
}

bool Device::connect() {
    if (!m_connector) {
        logging::get_log("main")->error("Device: connect was called but not connector was set up");
        return false;
    }

    return m_connector->connect();
}

void Device::disconnect() {
    if (!m_connector) {
        logging::get_log("main")->error("Device: disconnect was called but not connector was set up");
        return;
    }

    m_connector->disconnect();
}

void Device::send(const QByteArray &data) {
    if (!m_connector) {
        logging::get_log("main")->error("Device: send was called but not connector was set up; data to be sent is {0} (hex)",
                                        data.toHex().toStdString());
        return;
    }

    m_connector->write(data);
}

device_id Device::get_id() {
    static std::atomic<device_id> id{0};
    return ++id;
}