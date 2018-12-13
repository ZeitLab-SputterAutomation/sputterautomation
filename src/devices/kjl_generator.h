#pragma once

#include "device.h"
#include "rf_generator.h"

class KJLGenerator : public RFGenerator {
    Q_OBJECT
public:
    KJLGenerator() = default;
    KJLGenerator(std::unique_ptr<BaseConnector> &&connector);
    ~KJLGenerator() = default;

    KJLGenerator(const KJLGenerator &) = delete;
    KJLGenerator &operator=(const KJLGenerator &) = delete;

    // overrides from Device
    void set_connector(std::unique_ptr<BaseConnector> &&connector) override;
    void init(std::shared_ptr<config::Segment> settings) override;
    void update() override;

    // overrides from RFGenerator
    void output_on() override;
    void output_off() override;
    void set_target_power(int power) override;
    void set_load_capacitor_position(int position) override;
    void set_tune_capacitor_position(int position) override;

    void query_capacitor_positions() override;
    void query_external_feedback() override;
    void query_forward_power() override;
    void query_reflected_power() override;

    // Query the setpoint, forward power, reflected power, maximum power and additional status information from the generator
    void query_status();

private:
    void handle_data_received(const QByteArray &data);
    void handle_reply(const QByteArray &data);

    // Saves the received parameters from the generator. When all were received successfully, they are reported to the controller
    // and set to -1 again.
    struct {
        int external_feedback = -1;
        int forward_power = -1;
        int reflected_power = -1;
        int setpoint = -1;
        int load_cap_position = -1;
        int tune_cap_position = -1;
    } m_generator_parameters;
    std::mutex m_parameter_mutex;

    QByteArray m_in_buffer;
    std::mutex m_data_mutex;

    std::mutex m_command_mutex;
    std::list<QByteArray> m_command_queue;
};