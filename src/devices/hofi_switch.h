#pragma once

#include "device.h"
#include "rf_switch.h"

class HofiSwitch : public RFSwitch {
    Q_OBJECT
public:
    HofiSwitch() = default;
    HofiSwitch(std::unique_ptr<BaseConnector> &&connector);
    ~HofiSwitch() = default;

    HofiSwitch(const HofiSwitch &) = delete;
    HofiSwitch &operator=(const HofiSwitch &) = delete;

    // overrides from Device
    void set_connector(std::unique_ptr<BaseConnector> &&connector) override;
    void init(std::shared_ptr<config::Segment> settings) override;

    // overrides from RFSwitch
    void set_port(int8_t port) override;
    int8_t get_port() override;

private:
    void handle_data_received(const QByteArray &data);

    int8_t m_current_port = -1;
    QByteArray m_in_buffer;
    std::mutex m_data_mutex;
};