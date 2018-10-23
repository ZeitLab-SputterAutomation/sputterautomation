#include "serial_connector.h"

#include "logging/logging.h"

#include "util/util.h"

SerialConnector::SerialConnector() noexcept : BaseConnector("serial") {
    QObject::connect(&m_serial_port, &QSerialPort::readyRead, this, &SerialConnector::handle_ready_read);
}

SerialConnector::~SerialConnector() { disconnect(); }

void SerialConnector::init(std::shared_ptr<config::Segment> settings) {
    // Keep track if we were able to successfully change all settings (used in printing info about the connector)
    bool all_set = true;

    if (!settings) {
        logging::get_log("main")->error("SerialConnector: init called with nullptr");
        all_set = false;
        return;
    }

    if (auto baudrate = settings->get<int>("baudrate")) {
        if (!m_serial_port.setBaudRate(*baudrate)) {
            logging::get_log("main")->warn("SerialConnector: setBaudRate failed for value {0}", *baudrate);
            all_set = false;
        } else {
            m_settings.baudrate = *baudrate;
        }
    }

    if (auto databits = settings->get<QSerialPort::DataBits>("databits")) {
        if (!m_serial_port.setDataBits(*databits)) {
            logging::get_log("main")->warn("SerialConnector: setDataBits failed for value {0}", *databits);
            all_set = false;
        } else {
            m_settings.databits = *databits;
        }
    }

    if (auto parity = settings->get<QSerialPort::Parity>("parity")) {
        if (!m_serial_port.setParity(*parity)) {
            logging::get_log("main")->warn("SerialConnector: setParity failed for value {0}", *parity);
            all_set = false;
        } else {
            m_settings.parity = *parity;
        }
    }

    if (auto stopbits = settings->get<QSerialPort::StopBits>("stopbits")) {
        if (!m_serial_port.setStopBits(*stopbits)) {
            logging::get_log("main")->warn("SerialConnector: setStopBits failed for value {0}", *stopbits);
            all_set = false;
        } else {
            m_settings.stopbits = *stopbits;
        }
    }

    if (auto flowcontrol = settings->get<QSerialPort::FlowControl>("flowcontrol")) {
        if (!m_serial_port.setFlowControl(*flowcontrol)) {
            logging::get_log("main")->warn("SerialConnector: setFlowControl failed for value {0}", *flowcontrol);
            all_set = false;
        } else {
            m_settings.flowcontrol = *flowcontrol;
        }
    }

    if (auto portname = settings->get<std::string>("portname")) {
        m_serial_port.setPortName(QString::fromStdString(*portname));
        all_set = false;
    } else {
        m_settings.portname = *portname;
    }

    m_settings.all_set = all_set;
}

bool SerialConnector::connect() {
    if (!m_serial_port.open(QIODevice::ReadWrite)) {
        std::string error_string{""};

        if (m_serial_port.error() != QSerialPort::NoError) error_string = m_serial_port.errorString().toStdString();

        logging::get_log("main")->error("SerialPort: QSerialPort::open failed with error '{0}'", error_string);

        return false;
    }

    return true;
}

void SerialConnector::disconnect() {
    m_connected = false;

    m_serial_port.close();
}

void SerialConnector::write(const QByteArray &data) {
    if (data.length() == 0) return;

    if (!is_connected()) return;

    // TODO: handle the return value
    m_serial_port.write(data, data.length());
}

std::string SerialConnector::info() {
    if (!m_settings.all_set) {
        return "SerialConnector: This connector is in an error-state: not all settings "s
               "could be changed in the last call to init"s;
    }

    // clang-format off
    return fmt::format(
        "SerialConnector:\n\tportname: {1}\n\tdata bits: {2}\n\tparity: {3}\n\tstop bits: {4}\n\tflow control: {5}",
        m_settings.portname,
        util::qt_enum_to_string(m_settings.databits),
        util::qt_enum_to_string(m_settings.parity),
        util::qt_enum_to_string(m_settings.stopbits),
        util::qt_enum_to_string(m_settings.flowcontrol));
    // clang-format on
}

void SerialConnector::handle_ready_read() {
    QByteArray data = m_serial_port.readAll();

    emit data_received(data);
}