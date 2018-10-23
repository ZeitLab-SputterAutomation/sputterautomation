#pragma once

#include <string>

#include "config/segment.h"

#include <QObject>
#include <QByteArray>

class BaseConnector : public QObject {
    Q_OBJECT
public:
    BaseConnector(const std::string &type = "") noexcept : m_type{type} {};
    virtual ~BaseConnector() = default;

    BaseConnector(const BaseConnector &) = delete;
    BaseConnector &operator=(const BaseConnector &) = delete;

    virtual void init(std::shared_ptr<config::Segment> settings) {};

    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    bool is_connected() noexcept { return m_connected; }

    // Send data via the connector
    virtual void write(const QByteArray &data) = 0;

    // Returns general information about the connector, i.e. connection status, all settings, etc.
    virtual std::string info() = 0;

signals:
    // This signal gets emitted when data has been successfully received by the connector
    void data_received(QByteArray data);

protected:
    bool m_connected = false;

    // Describes the type of the connector, for example "serial" for a connector that transfers data via a serial
    // connection and "ethernet" for a connector transferring data via ethernet. This is needed to allow devices to
    // implement different protocols for different types of connectors.
    std::string m_type;
};
