#pragma once

// This file configures certain hard-coded application behaviour.

#include <string>
#include <string_view>
using namespace std::string_literals;
using namespace std::string_view_literals;

// Path to search for config files
constexpr const auto CONFIG_PATH = "./config/";
// Spaces used for indentation of each level in a segment during serialization
constexpr const auto CONFIG_SPACES_PER_INDENT_LEVEL = 4;

// Maximum number of retries to send commands to a device
constexpr const auto MAX_SEND_RETRIES = 10;

// Default maximum wait time for ethernet connections in ms
constexpr const auto MAX_ETHERNET_CONNECT_WAIT = 3000;
