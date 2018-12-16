#pragma once

#include "db.h"
#include "state.h"

#include "config/segment.h"
#include "devices/connector/connector.h"

// for logging speed and protocol types directly
#include "spdlog/fmt/bundled/ostream.h"

#include "nodave_wrapper.h"

#include <QObject>
#include <QTimer>

// TODO: implement name to enum value
namespace PLC {
    namespace Nodave {
        // clang-format off
    enum class speed : int_fast32_t {
        undefined   = -1,
        speed_9k    = daveSpeed9k,
        speed_19k   = daveSpeed19k,
        speed_187k  = daveSpeed187k,
        speed_500k  = daveSpeed500k,
        speed_1500k = daveSpeed1500k,
        speed_45k   = daveSpeed45k,
        speed_93k   = daveSpeed93k,
    };
    enum class protocol : int_fast32_t {
        undefined     = -1,
        MPI           = daveProtoMPI,
        MPI2          = daveProtoMPI2,
        MPI3          = daveProtoMPI3,
        MPI4          = daveProtoMPI4,
        PPI           = daveProtoPPI,
        AS511         = daveProtoAS511,
        S7online      = daveProtoS7online,
        ISOTCP        = daveProtoISOTCP,
        ISOTCP243     = daveProtoISOTCP243,
        MPI_IBH       = daveProtoMPI_IBH,
        PPI_IBH       = daveProtoPPI_IBH,
        NLpro         = daveProtoNLpro,
        UserTransport = daveProtoUserTransport
    };
        // clang-format on

        // Returns the name for the enum settings
        std::string get_name(speed s);
        std::string get_name(protocol p);

        // Overloads for operator<< to enable fmtlib to handle the enums
        std::ostream &operator<<(std::ostream &os, speed s);
        std::ostream &operator<<(std::ostream &os, protocol p);
    }  // namespace Nodave

    // The S7 class provides an interface to the S7 PLC by Siemens currently used. It is a more or less direct conversion from
    // the old software and needs a major revamp. For example, DBword and DBDataword (and their dword-variants) should be merged,
    // there is no need to keep them seperated.
    class S7 : public QObject {
        Q_OBJECT
    public:
        S7();
        ~S7();

        S7(const S7 &) = delete;
        S7 &operator=(const S7 &) = delete;

        void init(std::shared_ptr<config::Segment> settings);

        bool connect();
        void disconnect();

        void start();
        void stop();

        void set_dbword(std::string name, uint16_t data);
        void set_dbdword(std::string name, uint32_t data);
        uint16_t get_dbword(std::string name);
        uint32_t get_dbdword(std::string name);

        void set_flag(std::string name, bool state);
        void set_output(std::string name, bool state);
        bool get_flag(std::string name);
        bool get_input(std::string name);
        bool get_output(std::string name);

        void set_io(std::string name, bool state);
        bool get_io(std::string name);

    signals:
        void flag_changed(std::string name, bool state);
        void input_changed(std::string name, bool state);
        void output_changed(std::string name, bool state);

        void dbword_changed(std::string name, uint16_t data);
        void dbdword_changed(std::string name, uint32_t data);

    private:
        bool poll_flags();
        bool poll_inputs();
        bool poll_outputs();
        void poll_dbwords();
        void poll_dbdwords();

        void poll();

        struct {
            Nodave::speed speed = Nodave::speed::undefined;
            Nodave::protocol protocol = Nodave::protocol::undefined;
            std::string interface;
        } m_plc_settings;

        std::unique_ptr<EthernetConnector> m_connector;

        daveConnection *m_plc_connection = nullptr;
        daveInterface *m_plc_interface = nullptr;

        bool m_plc_connected = false;

        uint64_t m_flags = 0;
        uint64_t m_inputs = 0;
        uint64_t m_outputs = 0;

        std::unordered_map<std::string, State> m_flag_map;
        std::unordered_map<std::string, State> m_input_map;
        std::unordered_map<std::string, State> m_output_map;

        std::vector<DBDataword> m_db_datawords;
        std::vector<DBDatadword> m_db_datadwords;
        std::unordered_map<std::string, DBword> m_dbword_map;
        std::unordered_map<std::string, DBdword> m_dbdword_map;

        QTimer m_poll_timer;
    };
}  // namespace PLC