#pragma once

#include <memory>

#include "connector/connector.h"
#include "config/segment.h"
#include "util/util.h"

class Device : public QObject {
    Q_OBJECT
public:
    Device() = default;
    Device(std::unique_ptr<BaseConnector> &&connector);
    virtual ~Device() = default;

    Device(const Device &) = delete;
    Device &operator=(const Device &) = delete;

    virtual void set_connector(std::unique_ptr<BaseConnector> &&connector);

    virtual void init(std::shared_ptr<config::Segment> settings){};

    // This function is called periodically by the controller. Use it for example to update internal state machines.
    virtual void update(){};

    // connection control
    bool is_connected();
    bool connect();
    void disconnect();

    // connector control
    void send(const QByteArray &data);

    template <typename... Ts>
    void send(const Ts... args) {
        send(util::make_byte_array(args...));
    }

    // general device control

    // Turn the device on or off. Note that these functions should not be used to control an output; implement those in a suitable
    // base class, for example output_on/off in RFGenerator.
    virtual void turn_on(){};
    virtual void turn_off(){};
    virtual bool is_on() { return false; }

    // Sets the control mode of the device, for example to remote, local or mixed control
    enum class ControlMode : uint8_t { Remote = 0b1, Local = 0b10 };
    virtual void set_control_mode(ControlMode /*mode*/) {}

signals:
    void connection_status_changed();

protected:
    BaseConnector *get_connector() { return m_connector.get(); }

    std::unique_ptr<BaseConnector> m_connector;
};
