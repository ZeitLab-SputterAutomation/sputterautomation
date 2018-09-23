#pragma once

#include "base_connector.h"

#include <queue>

#include <QByteArray>
#include <QSerialPort>

class SerialConnector : public BaseConnector {
    Q_OBJECT
public:
    SerialConnector();
    ~SerialConnector();

    void init(std::shared_ptr<config::Segment> settings) noexcept override;

    bool connect() override;
    void disconnect() override;

    void write(const QByteArray &data) override;

private:
    void handle_ready_read();

    std::string m_ip_address{""};
    int m_port{0};

    QSerialPort m_serial_port;
};