#pragma once

#include "device.h"

class RFGenerator : public Device {
    Q_OBJECT
public:
    RFGenerator() = default;
    RFGenerator(std::unique_ptr<BaseConnector> &&connector) : Device(std::move(connector)){};
    virtual ~RFGenerator() = default;

    RFGenerator(const RFGenerator &) = delete;
    RFGenerator &operator=(const RFGenerator &) = delete;

    // power control
    virtual void output_on() = 0;
    virtual void output_off() = 0;
    virtual void set_target_power(int power) = 0;

    // matching network control
    virtual void set_load_capacitor_position(int /*position*/){};
    virtual void set_tune_capacitor_position(int /*position*/){};

    enum class MatchnetworkMode { Automatic, Manual };
    // Sets the behavior of the matching network to either automatic matching or manual matching.
    virtual void set_matchnetwork_mode(MatchnetworkMode /*mode*/){};  // manual, automatic, ...

    // matching network queries
    virtual void query_capacitor_positions(){};
    virtual void query_external_feedback(){};
    virtual void query_forward_power(){};
    virtual void query_reflected_power(){};

signals:
    void update_parameters();
};
