#pragma once

#include "device.h"

class RFSwitch : public Device {
    Q_OBJECT
public:
    RFSwitch() = default;
    RFSwitch(std::unique_ptr<BaseConnector> &&connector) : Device(std::move(connector)) {}
    virtual ~RFSwitch() = default;

    RFSwitch(const RFSwitch &) = delete;
    RFSwitch &operator=(const RFSwitch &) = delete;

    virtual void set_port(int8_t port) = 0;
    virtual int8_t get_port() = 0;

signals:
    void port_changed(int8_t port);
};
