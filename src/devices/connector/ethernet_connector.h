#pragma once

#include "app_config.h"
#include "base_connector.h"

#include <QTcpSocket>

class EthernetConnector : public BaseConnector {
    Q_OBJECT
public:
    EthernetConnector() noexcept;
    ~EthernetConnector();

    EthernetConnector(const EthernetConnector &) = delete;
    EthernetConnector &operator=(const EthernetConnector &) = delete;

    void init(std::shared_ptr<config::Segment> settings) noexcept override;

    // Try to establish a connection. Returns true on success and false otherwise.
    bool connect() override;
    void disconnect() override;

    // Writes the given data to the connection, discarding it silently when the connection has not been established yet.
    void write(const QByteArray &data) override;

    // Returns general information about the connector, i.e. connection status, all settings, etc.
    std::string info() override;

    // Move the underlying connection to the given thread.
    void move_to_thread(QThread *thread) override;

    // Returns the descriptor of the underlying socket.
    int get_descriptor();

private:
    void handle_ready_read();

    std::string m_ip_address;
    int m_port = 0;
    int m_max_connect_wait = MAX_ETHERNET_CONNECT_WAIT;

    QTcpSocket m_socket;
};