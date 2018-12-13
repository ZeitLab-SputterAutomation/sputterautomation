#pragma once

#include "serial_connector.h"
#include "ethernet_connector.h"

#include "app_config.h"

inline std::unique_ptr<BaseConnector> make_connector(std::string_view type) {
    if (type == "serial"sv) {
        return std::make_unique<SerialConnector>();
    } else if (type == "ethernet"sv) {
        return std::make_unique<EthernetConnector>();
    }

    logging::get_log("main")->warn("make_connector: unknown type {0}", type);

    return nullptr;
}