#pragma once

#include "device.h"
#include "rf_generator.h"

#include <list>
#include <mutex>

#include "util/util.h"

class CesarGenerator : public RFGenerator {
    Q_OBJECT
public:
    CesarGenerator() = default;
    CesarGenerator(std::unique_ptr<BaseConnector> &&connector);
    ~CesarGenerator() = default;

    CesarGenerator(const CesarGenerator &) = delete;
    CesarGenerator &operator=(const CesarGenerator &) = delete;

    // overrides from Device
    void set_connector(std::unique_ptr<BaseConnector> &&connector) override;
    void init(std::shared_ptr<config::Segment> settings) override;
    void update() override;
    virtual void set_control_mode(ControlMode mode) override;

    // overrides from RFGenerator
    void output_on() override;
    void output_off() override;
    void set_target_power(int power) override;
    void set_load_capacitor_position(int position) override;
    void set_tune_capacitor_position(int position) override;

    void set_matchnetwork_mode(MatchnetworkMode mode) override;

    void query_capacitor_positions() override;
    void query_external_feedback() override;
    void query_forward_power() override;
    void query_reflected_power() override;

private:
    // Queue a command to be sent to the device. If no other commands are queued it will be sent immediately. Otherwise
    // the next command is sent when a valid reply was received.
    void queue_command(QByteArray command);
    
    // Since QByteArray does not provide a constructor taking an initializer_list, we use this little templated
    // queue_command to simplify the construction of packages.
    // This overload takes any number of parameters and packs them in one QByteArray, which is passed on to queue_command.
    template <typename... Ts>
    constexpr void queue_command(const Ts... args) {
        // static_assert(util::all_true<std::is_convertible_v<Ts, uint8_t>...>::value,
        //              "all values in the call to queue_command must be convertible to uint8_t");

        queue_command(util::make_byte_array(args...));
    }

    // Send the first command in the queue to the device. Returns true when a command was sent and false otherwise (i.e.
    // when the queue is empty).
    bool write_command();

    // Handle new data received via the connector. If not enough data is received it will be buffered and handled once
    // the full package arrived.
    void handle_data_received(const QByteArray &data);

    // Packet is used to save the decoded command number and additional data of a received command reply.
    struct Packet {
        uint8_t command;
        QByteArray data;
    };

    // Handle a received reply, i.e. notify the correct places about the data received.
    void handle_reply(const Packet &packet);

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

    std::mutex m_data_mutex;
    QByteArray m_in_buffer;

    // Address of the generator (called "busaddress" in the old software)
    int m_address = 0;

    int m_send_retries = 0;
    std::mutex m_command_mutex;
    std::list<QByteArray> m_command_queue;

    // clang-format off
    enum class Commands : uint8_t {
        OutputOff = 1,
        OutputOn = 2,
        SetPowerSetPoint = 8,                    // 0x8
        SetMatchNetworkControl = 13,             // 0xD
        SelectActiveControlMode = 14,            // 0xE
        SetReflectedPowerParameters = 33,        // 0x21
        ErrorMatchingNetworkNotConnected = 53,   // Listed as command to use it in the switch statement
        MoveLoadCapPosition = 112,               // 0x70
        MoveTuneCapPosition = 122,               // 0x7A
        //ReportPowerSupplyType = 128,           // 0x80, not sent
        //ReportMatchNetworkMotorMovement = 131, // 0x83, not sent
        //ReportReflectedPowerParameters = 152,  // 0x98, not sent
        //ReportRegulationMode = 154,            // 0x9A, not sent
        //ReportActiveControlMode = 155,         // 0x9B, not sent
        //ReportProcessStatus = 162,             // 0xA2, not sent
        ReportSetPointAndRegulationMode = 164,   // 0xA4
        ReportForwardPower = 165,                // 0xA5
        ReportReflectedPower = 166,              // 0xA6
        //ReportDeliveredPower = 167,            // 0xA7, not sent
        ReportExternalFeedback = 168,            // 0xA8
        //ReportForwardPowerLimit = 169,         // 0xA9, not sent
        //ReportReflectedPowerLimit = 170,       // 0xAA, not sent
        ReportCapacitorPositions = 175,          // 0xAF
        //ReportUnitRunTime = 205,               // 0xCD, not sent
        ReportFaultStatusRegister = 223,         // 0xDF
    };
    // clang-format on
    const static uint8_t ACK = 0x6;
    const static uint8_t NACK = 0x15;
};
