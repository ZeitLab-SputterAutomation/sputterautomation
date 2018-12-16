#include "s7.h"

#include "stacktrace.h"
#include "util/util.h"

#include <unordered_map>

namespace PLC {

    namespace Nodave {
        std::string get_name(speed s) {
            static std::unordered_map<speed, std::string> names = {
                {speed::undefined, "undefined"},   {speed::speed_9k, "speed_9k"},     {speed::speed_19k, "speed_19k"},
                {speed::speed_187k, "speed_187k"}, {speed::speed_500k, "speed_500k"}, {speed::speed_1500k, "speed_1500k"},
                {speed::speed_45k, "speed_45k"},   {speed::speed_93k, "speed_93k"}};

            return names[s];
        }

        std::string get_name(protocol p) {
            static std::unordered_map<protocol, std::string> names = {
                {protocol::undefined, "undefined"}, {protocol::MPI, "MPI"},
                {protocol::MPI2, "MPI2"},           {protocol::MPI3, "MPI3"},
                {protocol::MPI4, "MPI4"},           {protocol::PPI, "PPI"},
                {protocol::AS511, "AS511"},         {protocol::S7online, "S7online"},
                {protocol::ISOTCP, "ISOTCP"},       {protocol::ISOTCP243, "ISOTCP243"},
                {protocol::MPI_IBH, "MPI_IBH"},     {protocol::PPI_IBH, "PPI_IBH"},
                {protocol::NLpro, "NLpro"},         {protocol::UserTransport, "UserTransport"}};

            return names[p];
        }

        std::ostream &operator<<(std::ostream &os, speed s) { return os << get_name(s); }

        std::ostream &operator<<(std::ostream &os, protocol p) { return os << get_name(p); }
    }  // namespace Nodave

    S7::S7() {
        m_connector = std::make_unique<EthernetConnector>();

        // Default poll intervall of 1000ms
        m_poll_timer.setInterval(1000);

        QObject::connect(&m_poll_timer, &QTimer::timeout, this, &S7::poll);
    }

    S7::~S7() { disconnect(); }

    void S7::init(std::shared_ptr<config::Segment> settings) {
        // Let the connector load its settings
        if (auto connector_settings = settings->get_segment("connection")) {
            m_connector->init(connector_settings);
        }

        // Load our settings
        if (auto speed = settings->get<Nodave::speed>("speed")) {
            m_plc_settings.speed = *speed;
        }

        if (auto protocol = settings->get<Nodave::protocol>("protocol")) {
            m_plc_settings.protocol = *protocol;
        }

        if (auto interface = settings->get<std::string>("interface")) {
            m_plc_settings.interface = *interface;
        }

        if (auto pollinterval = settings->get<int>("pollinterval")) {
            m_poll_timer.setInterval(*pollinterval);
        }

        // Load flags
        auto flags = settings->get_all("flags");
        for (const auto &flag : flags) {
            auto tokens = *util::split(flag.second, '.', false);
            if (tokens.size() != 2) {
                logging::get_log("main")->warn("S7: wrong flag address format encountered, got '{0}'", flag.second);
                continue;
            }

            auto addr_high = util::to_type<int>(tokens[0]);
            auto addr_low = util::to_type<int>(tokens[1]);
            if (!addr_high || !addr_low) {
                logging::get_log("main")->warn("S7: non-integer flag address part encountered, got '{0}' for flag '{1}'",
                                               flag.second, flag.first);
                continue;
            }

            m_flag_map[flag.first].address = *addr_high * 8 + *addr_low;
        }

        // Load inputs
        auto inputs = settings->get_all("inputs");
        for (const auto &input : inputs) {
            auto tokens = *util::split(input.second, '.', false);
            if (tokens.size() != 2) {
                logging::get_log("main")->warn("S7: wrong input address format encountered, got '{0}'", input.second);
                continue;
            }

            auto addr_high = util::to_type<int>(tokens[0]);
            auto addr_low = util::to_type<int>(tokens[1]);
            if (!addr_high || !addr_low) {
                logging::get_log("main")->warn("S7: non-integer input address part encountered, got '{0}' for input '{1}'",
                                               input.second, input.first);
                continue;
            }

            m_input_map[input.first].address = *addr_high * 8 + *addr_low;
        }

        // Load outputs
        auto outputs = settings->get_all("outputs");
        for (const auto &output : outputs) {
            auto tokens = *util::split(output.second, '.', false);
            if (tokens.size() != 2) {
                logging::get_log("main")->warn("S7: wrong flag address format encountered, got '{0}'", output.second);
                continue;
            }

            auto addr_high = util::to_type<int>(tokens[0]);
            auto addr_low = util::to_type<int>(tokens[1]);
            if (!addr_high || !addr_low) {
                logging::get_log("main")->warn("S7: non-integer output address part encountered, got '{0}' for output '{1}'",
                                               output.second, output.first);
                continue;
            }

            m_output_map[output.first].address = *addr_high * 8 + *addr_low;
        }

        // Load dbwords
        // We also need to find the highest length (address_data/2 + 1) for all address_db's. We do this by using a map that maps
        // the address_db to the address_data.
        std::unordered_map<int, int> addresses;

        auto dbwords = settings->get_all("dbwords");
        for (const auto &dbword : dbwords) {
            auto tokens = *util::split(dbword.second, '.', false);
            if (tokens.size() != 2) {
                logging::get_log("main")->warn("S7: wrong dbword address format encountered, got '{0}'", dbword.second);
                continue;
            }

            auto addr_db = util::to_type<int>(tokens[0]);
            auto addr_data = util::to_type<int>(tokens[1]);
            if (!addr_db || !addr_data) {
                logging::get_log("main")->warn("S7: non-integer dbword address part encountered, got '{0}' for dbword '{1}'",
                                               dbword.second, dbword.first);
                continue;
            }

            m_dbword_map[dbword.first].address_db = *addr_db;
            m_dbword_map[dbword.first].address_data = *addr_data;

            if (addresses.count(*addr_db) == 0) {
                addresses[*addr_db] = *addr_data;
            } else if (addresses[*addr_db] < *addr_data) {
                addresses[*addr_db] = *addr_data;
            }
        }

        for (const auto &entry : addresses) {
            m_db_datawords.push_back(DBDataword(entry.second));
        }
        addresses.clear();

        // Load dbdwords
        // Same as above
        auto dbdwords = settings->get_all("dbdwords");
        for (const auto &dbdword : dbdwords) {
            auto tokens = *util::split(dbdword.second, '.', false);
            if (tokens.size() != 2) {
                logging::get_log("main")->warn("S7: wrong dbdword address format encountered, got '{0}'", dbdword.second);
                continue;
            }

            auto addr_db = util::to_type<int>(tokens[0]);
            auto addr_data = util::to_type<int>(tokens[1]);
            if (!addr_db || !addr_data) {
                logging::get_log("main")->warn("S7: non-integer dbdword address part encountered, got '{0}' for dbdword '{1}'",
                                               dbdword.second, dbdword.first);
                continue;
            }

            m_dbdword_map[dbdword.first].address_db = *addr_db;
            m_dbdword_map[dbdword.first].address_data = *addr_data;

            if (addresses.count(*addr_db) == 0) {
                addresses[*addr_db] = *addr_data;
            } else if (addresses[*addr_db] < *addr_data) {
                addresses[*addr_db] = *addr_data;
            }
        }

        for (const auto &entry : addresses) {
            m_db_datadwords.push_back(DBDatadword(entry.second));
        }
    }

    bool S7::connect() {
        if (m_plc_settings.protocol == Nodave::protocol::undefined || m_plc_settings.speed == Nodave::speed::undefined
            || m_plc_settings.interface.empty()) {
            logging::get_log("main")->error(
                "S7: not all settings were set up. Current values:\n\tprotocol: {0}\n\tspeed: {1}\n\tinterface: '{2}'",
                m_plc_settings.protocol, m_plc_settings.speed, m_plc_settings.interface);
            return false;
        }

        if (!m_connector->connect()) {
            logging::get_log("main")->error("S7: unable to connect to the PLC. Connector info:\n  {0}", m_connector->info());
            return false;
        } else {
            logging::get_log("main")->debug("S7: connected. Connector info:\n  {0}", m_connector->info());
        }

        int tcp_descriptor = m_connector->get_descriptor();

#pragma warning(suppress : 4312)  // warning C4312: 'reinterpret_cast': conversion from 'int' to 'HANDLE' of greater size
        _daveOSserialType serial_type{reinterpret_cast<HANDLE>(tcp_descriptor), reinterpret_cast<HANDLE>(tcp_descriptor)};

        m_plc_interface = daveNewInterface(serial_type, const_cast<char *>(m_plc_settings.interface.c_str()), 0,
                                           static_cast<int>(m_plc_settings.protocol), static_cast<int>(m_plc_settings.speed));

        // Currently daveNewInterface fails only if calloc fails
        if (!m_plc_interface) {
            logging::get_log("main")->error("S7: interface generation failed");
            return false;
        }

        daveSetTimeout(m_plc_interface, 5000000);
        m_plc_connection = daveNewConnection(m_plc_interface, 0, 0, 0);

        if (int ret = daveConnectPLC(m_plc_connection); ret != daveResOK) {
            logging::get_log("main")->error("S7: connection to PCL failed: {0}", std::string(daveStrerror(ret)));
            return false;
        }

        m_plc_connected = true;

        logging::get_log("main")->debug("S7: connection successful");
        return true;
    }

    void S7::disconnect() {
        if (m_plc_connection) {
            daveDisconnectPLC(m_plc_connection);
            daveFree(m_plc_connection);
            m_plc_connection = nullptr;
        }

        if (m_plc_interface) {
            daveDisconnectAdapter(m_plc_interface);
            daveFree(m_plc_interface);
            m_plc_interface = nullptr;
        }

        m_plc_connected = false;
    }

    void S7::start() {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to start: not connected to the S7");
            return;
        }

        m_poll_timer.start();

        logging::get_log("main")->debug("S7 timer started");
    }

    void S7::stop() { m_poll_timer.stop(); }

    void S7::set_dbword(std::string name, uint16_t data) {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to set db-value {0}: not connected to the S7", name);
            return;
        }

        if (m_dbword_map.count(name) == 0) {
            logging::get_log("main")->error("S7: unknown DBword {0}", name);
            return;
        }

        DBword db = m_dbword_map[name];
        data = util::convert_endian(data);

        if (int ret = daveWriteBytes(m_plc_connection, daveDB, db.address_db, db.address_data, 2, &data); ret != daveResOK) {
            logging::get_log("main")->error(
                "S7: writing DBword failed:\n\tname: {0}\n\tDB address: {1}\n\tvalue address: {2}\n\terror: {3}", name,
                db.address_db, db.address_data, std::string(daveStrerror(ret)));
        }
    }

    void S7::set_dbdword(std::string name, uint32_t data) {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to set db-value {0}: not connected to the S7", name);
            return;
        }

        if (m_dbdword_map.count(name) == 0) {
            logging::get_log("main")->error("S7: unknown DBword {0}", name);
            return;
        }

        DBdword db = m_dbdword_map[name];
        data = util::convert_endian(data);

        if (int ret = daveWriteBytes(m_plc_connection, daveDB, db.address_db, db.address_data, 4, &data); ret != daveResOK) {
            logging::get_log("main")->error(
                "S7: writing DBdword failed:\n\tname: {0}\n\tDB address: {1}\n\tvalue address: {2}\n\terror: {3}", name,
                db.address_db, db.address_data, std::string(daveStrerror(ret)));
        }
    }

    uint16_t S7::get_dbword(std::string name) {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to get db-value {0}: not connected to the S7", name);
            return 0;
        }

        if (m_dbword_map.count(name) == 0) {
            logging::get_log("main")->error("S7: unknown DBword {0}", name);
            return 0;
        }

        return m_dbword_map[name].data;
    }

    uint32_t S7::get_dbdword(std::string name) {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to get db-value {0}: not connected to the S7", name);
            return 0;
        }

        if (m_dbdword_map.count(name) == 0) {
            logging::get_log("main")->error("S7: unknown DBdword {0}", name);
            return 0;
        }

        return m_dbdword_map[name].data;
    }

    void S7::set_flag(std::string name, bool state) {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to set flag {0}: not connected to the S7", name);
            return;
        }

        if (m_flag_map.count(name) == 0) {
            logging::get_log("main")->error("S7: unknown flag {0}", name);
            return;
        }

        if (!poll_flags()) {
            logging::get_log("main")->warn("S7: unable to set flag {0}: unable to poll flags", name);
            return;
        }

        uint64_t states = m_flags;
        if (state) {
            states |= 1i64 << m_flag_map[name].address;
        } else {
            states &= ~(1i64 << m_flag_map[name].address);
        }
        if (int ret = daveWriteBytes(m_plc_connection, daveFlags, 0, 0, 6, &states); ret != daveResOK) {
            logging::get_log("main")->error("S7: writing flag failed:\n\tname: {0}\n\taddress: {1}\n\terror: {2}", name,
                                            m_flag_map[name].address, std::string(daveStrerror(ret)));
        }
    }

    void S7::set_output(std::string name, bool state) {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to set output {0}: not connected to the S7", name);
            return;
        }

        if (m_output_map.count(name) == 0) {
            logging::get_log("main")->error("S7: unknown output {0}", name);
            return;
        }

        if (!poll_outputs()) {
            logging::get_log("main")->warn("S7: unable to set output {0}: unable to poll outputs", name);
            return;
        }

        uint64_t states = m_outputs;
        if (state) {
            states |= 1i64 << m_output_map[name].address;
        } else {
            states &= ~(1i64 << m_output_map[name].address);
        }
        if (int ret = daveWriteBytes(m_plc_connection, daveOutputs, 0, 0, 6, &states); ret != daveResOK) {
            logging::get_log("main")->error("S7: writing output failed:\n\tname: {0}\n\taddress: {1}\n\terror: {2}", name,
                                            m_flag_map[name].address, std::string(daveStrerror(ret)));
        }
    }

    bool S7::get_flag(std::string name) {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to get flag {0}: not connected to the S7", name);
            return false;
        }

        if (m_flag_map.count(name) == 0) {
            logging::get_log("main")->error("S7: unknown flag {0}", name);
            return false;
        }

        return m_flag_map[name].state;
    }

    bool S7::get_input(std::string name) {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to get input {0}: not connected to the S7", name);
            return false;
        }

        if (m_input_map.count(name) == 0) {
            logging::get_log("main")->error("S7: unknown input {0}", name);
            return false;
        }

        return m_input_map[name].state;
    }

    bool S7::get_output(std::string name) {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to get output {0}: not connected to the S7", name);
            return false;
        }

        if (m_output_map.count(name) == 0) {
            logging::get_log("main")->error("S7: unknown output {0}", name);
            return false;
        }

        return m_output_map[name].state;
    }

    // clang-format off
    [[deprecated("This is only kept for direct translation of the old version, replace it with set_flag/_output")]]
    void S7::set_io(std::string name, bool state) {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to set io {0}: not connected to the S7", name);
            return;
        }

        if (m_output_map.count(name) != 0) {
            set_output(name, state);
            return;
        }

        if (m_flag_map.count(name) != 0) {
            set_flag(name, state);
            return;
        }

        logging::get_log("main")->error("S7: unknown flag or output {0}", name);
    }
    
    [[deprecated("This is only kept for direct translation of the old version, replace it with get_input/_flag/_output")]]
    bool S7::get_io(std::string name) {
        bool state = false;

        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to get io {0}: not connected to the S7", name);
            return false;
        }

        if (m_flag_map.count(name) != 0) {
            state = get_flag(name);
        } else if (m_input_map.count(name) != 0) {
            state = get_input(name);
        } else if (m_output_map.count(name) != 0) {
            state = get_output(name);
        }

        return state;
    }
    // clang-format on

    bool S7::poll_flags() {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to poll flags: not connected to the S7");
            return false;
        }

        uint64_t flags = 0;
        if (int ret = daveReadBytes(m_plc_connection, daveFlags, 0, 0, 6, &flags); ret != daveResOK) {
            logging::get_log("main")->error("S7: polling flags failed with error {0}", std::string(daveStrerror(ret)));
            return false;
        }

        if (flags != m_flags) {
            m_flags = flags;

            // Iterate over all flags and see if they have changed..
            for (auto &flag : m_flag_map) {
                bool new_state = flags & (1i64 << flag.second.address);
                if (flag.second.state != new_state) {
                    flag.second.state = new_state;
                    // ..and if they did, notify listeners
                    emit flag_changed(flag.first, new_state);
                }
            }
        }

        return true;
    }

    bool S7::poll_inputs() {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to poll inputs: not connected to the S7");
            return false;
        }

        uint64_t inputs = 0;
        if (int ret = daveReadBytes(m_plc_connection, daveInputs, 0, 0, 6, &inputs); ret != daveResOK) {
            logging::get_log("main")->error("S7: polling inputs failed with error {0}", std::string(daveStrerror(ret)));
            return false;
        }

        if (inputs != m_inputs) {
            m_inputs = inputs;

            // Iterate over all inputs and see if they have changed..
            for (auto &input : m_flag_map) {
                bool new_state = inputs & (1i64 << input.second.address);
                if (input.second.state != new_state) {
                    input.second.state = new_state;
                    // ..and if they did, notify listeners
                    emit input_changed(input.first, new_state);
                }
            }
        }

        return true;
    }

    bool S7::poll_outputs() {
        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to poll outputs: not connected to the S7");
            return false;
        }

        uint64_t outputs = 0;
        if (int ret = daveReadBytes(m_plc_connection, daveOutputs, 0, 0, 6, &outputs); ret != daveResOK) {
            logging::get_log("main")->error("S7: polling outputs failed with error {0}", std::string(daveStrerror(ret)));
            return false;
        }

        if (outputs != m_outputs) {
            m_outputs = outputs;

            // Iterate over all outputs and see if they have changed..
            for (auto &output : m_output_map) {
                bool new_state = outputs & (1i64 << output.second.address);
                if (output.second.state != new_state) {
                    output.second.state = new_state;
                    // ..and if they did, notify listeners
                    emit output_changed(output.first, new_state);
                }
            }
        }

        return true;
    }

    void S7::poll_dbwords() {
        for (auto &db : m_db_datawords) {
            bool changed = false;

            std::vector<uint16_t> data(db.length);
            if (auto ret = daveReadBytes(m_plc_connection, daveDB, db.address, 0, db.length * 2, data.data()); ret != daveResOK) {
                logging::get_log("main")->error("S7: polling dbwords failed with error {0}", std::string(daveStrerror(ret)));
                return;
            }

            // Search for changed data
            for (size_t i = 0; i < db.length; i++) {
                data[i] = util::convert_endian(data[i]);
                if (data[i] != db.data[i]) {
                    db.data[i] = data[i];
                    changed = true;
                }
            }

            if (changed) {
                // Search for the corresponding entry in m_dbword_map and change the value
                for (auto &dbmap : m_dbword_map) {
                    if (dbmap.second.address_db == db.address) {
                        int address_data = dbmap.second.address_data / 2;
                        if (dbmap.second.data != data[address_data]) {
                            dbmap.second.data = data[address_data];

                            // Notify listeners about the changed dbword
                            emit dbword_changed(dbmap.second.name, dbmap.second.data);
                        }
                    }
                }
            }
        }
    }

    void S7::poll_dbdwords() {
        for (auto &db : m_db_datadwords) {
            bool changed = false;

            std::vector<uint32_t> data(db.length);
            if (auto ret = daveReadBytes(m_plc_connection, daveDB, db.address, 0, db.length * 4, data.data()); ret != daveResOK) {
                logging::get_log("main")->error("S7: polling dbdwords failed with error {0}", std::string(daveStrerror(ret)));
                return;
            }

            // Search for changed data
            for (size_t i = 0; i < db.length; i++) {
                data[i] = util::convert_endian(data[i]);
                if (data[i] != db.data[i]) {
                    db.data[i] = data[i];
                    changed = true;
                }
            }

            if (changed) {
                // Search for the corresponding entry in m_dbdword_map and change the value
                for (auto &dbmap : m_dbdword_map) {
                    if (dbmap.second.address_db == db.address) {
                        int address_data = dbmap.second.address_data / 4;
                        if (dbmap.second.data != data[address_data]) {
                            dbmap.second.data = data[address_data];

                            // Notify listeners about the changed dbdword
                            emit dbdword_changed(dbmap.second.name, dbmap.second.data);
                        }
                    }
                }
            }
        }
    }

    void S7::poll() {
        // TODO: remove this for production!
        m_poll_timer.stop();
        logging::get_log("main")->debug("S7 poll timer stopped (remove this)");

        if (!m_plc_connected) {
            logging::get_log("main")->error("S7: unable to poll: not connected to the S7");
            return;
        }

        poll_flags();
        poll_inputs();
        poll_outputs();

        poll_dbwords();
        poll_dbdwords();
    }
}  // namespace PLC