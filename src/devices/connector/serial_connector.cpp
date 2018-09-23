#include "serial_connector.h"

#include "logging/logging.h"

SerialConnector::SerialConnector() : BaseConnector("serial") {
    QObject::connect(&m_serial_port, &QSerialPort::readyRead, this, &SerialConnector::handle_ready_read);
}

SerialConnector::~SerialConnector() { disconnect(); }

void SerialConnector::init(std::shared_ptr<config::Segment> settings) noexcept {
    if (!settings) {
        logging::get_log("main")->error("SerialConnector: init called with nullptr");
        return;
    }

    if (auto baudrate = settings->get<int>("baudrate")) {
        if (!m_serial_port.setBaudRate(*baudrate)) {
            logging::get_log("main")->warn("SerialConnector: setBaudRate failed for value {0}", *baudrate);
        }
    }

    if (auto databits = settings->get<QSerialPort::DataBits>("databits")) {
        if (!m_serial_port.setDataBits(*databits)) {
            logging::get_log("main")->warn("SerialConnector: setDataBits failed for value {0}", *databits);
        }
    }

    if (auto parity = settings->get<QSerialPort::Parity>("parity")) {
        if (!m_serial_port.setParity(*parity)) {
            logging::get_log("main")->warn("SerialConnector: setParity failed for value {0}", *parity);
        }
    }

    if (auto stopbits = settings->get<QSerialPort::StopBits>("stopbits")) {
        if (!m_serial_port.setStopBits(*stopbits)) {
            logging::get_log("main")->warn("SerialConnector: setStopBits failed for value {0}", *stopbits);
        }
    }

    if (auto flowcontrol = settings->get<QSerialPort::FlowControl>("flowcontrol")) {
        if (!m_serial_port.setFlowControl(*flowcontrol)) {
            logging::get_log("main")->warn("SerialConnector: setFlowControl failed for value {0}", *flowcontrol);
        }
    }

    if (auto portname = settings->get<std::string>("portname")) {
        m_serial_port.setPortName(QString::fromStdString(*portname));
    }
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

    m_serial_port.write(data, data.length());
}

void SerialConnector::handle_ready_read() {
    QByteArray data = m_serial_port.readAll();

    emit data_received(data);
}