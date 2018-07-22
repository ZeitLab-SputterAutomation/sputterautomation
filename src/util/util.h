#pragma once
#include "app_config.h"

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "util_strings.h"

#if CHAR_BIT != 8
#error CHAR_BIT must be 8 for the endian conversion to work properly
#endif

namespace util {
    uint16_t convertEndian(uint16_t data);
    uint32_t convertEndian(uint32_t data);
    uint64_t convertEndian(uint64_t data);
}  // namespace util
