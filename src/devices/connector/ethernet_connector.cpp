#include "ethernet_connector.h"

EthernetConnector::EthernetConnector() noexcept : BaseConnector("ethernet") {
    QObject::connect(&m_socket, &QTcpSocket::readyRead, this, &EthernetConnector::handle_ready_read);
}

EthernetConnector::~EthernetConnector() { disconnect(); }

void EthernetConnector::init(std::shared_ptr<config::Segment> settings) noexcept {
    if (!settings) {
        logging::get_log("main")->error("EthernetConnector: init called with nullptr");
        return;
    }

    if (auto address = settings->get<std::string>("address")) {
        m_ip_address = *address;
    }

    if (auto port = settings->get<int>("port")) {
        m_port = *port;
    }

    if (auto waittime = settings->get<int>("waittime")) {
        m_max_connect_wait = *waittime;
    }
}

bool EthernetConnector::connect() {
    m_socket.connectToHost(QString::fromStdString(m_ip_address), m_port);

    if (!m_socket.waitForConnected(m_max_connect_wait)) {
        logging::get_log("main")->error("EthernetConnector: connection error: {0}", m_socket.errorString().toStdString());

        m_connected = false;
        return false;
    }

    m_connected = true;
    return true;
}

void EthernetConnector::disconnect() {
    m_connected = false;

    m_socket.close();
}

void EthernetConnector::write(const QByteArray &data) {
    if (data.length() == 0) return;

    if (!is_connected()) return;

    // TODO: handle the return value
    m_socket.write(data);
}

std::string EthernetConnector::info() {
    return fmt::format("EthernetConnector:\n\tIP Address: {0}\n\tPort: {1}", (m_ip_address.empty() ? "<not set>"s : m_ip_address),
                       m_port);
}

void EthernetConnector::move_to_thread(QThread *thread) { m_socket.moveToThread(thread); }

int EthernetConnector::get_descriptor() { return static_cast<int>(m_socket.socketDescriptor()); }

void EthernetConnector::handle_ready_read() {
    QByteArray data = m_socket.readAll();

    emit data_received(data);
}