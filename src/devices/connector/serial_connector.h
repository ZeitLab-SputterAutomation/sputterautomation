#pragma once

#include "base_connector.h"

#include <queue>

#include <QByteArray>
#include <QSerialPort>

class SerialConnector : public BaseConnector {
    Q_OBJECT
public:
    SerialConnector() noexcept;
    ~SerialConnector();

    void init(std::shared_ptr<config::Segment> settings) override;

    bool connect() override;
    void disconnect() override;

    void write(const QByteArray &data) override;

    std::string info() override;

private:
    void handle_ready_read();

    // This struct saves all settings so we can print them in debug messages. The default values were chosen
    // arbitrarily, so we track a successful setting of all settings with all_set. If it is false, at least one value
    // was not successfully set in the init method.
    struct {
        int baudrate = 0;
        std::string portname = ""s;
        QSerialPort::DataBits databits = QSerialPort::DataBits::Data8;
        QSerialPort::Parity parity = QSerialPort::Parity::NoParity;
        QSerialPort::StopBits stopbits = QSerialPort::StopBits::OneStop;
        QSerialPort::FlowControl flowcontrol = QSerialPort::FlowControl::NoFlowControl;
        bool all_set = false;
    } m_settings;

    QSerialPort m_serial_port;
};