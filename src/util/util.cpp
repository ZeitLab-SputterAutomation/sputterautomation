#include "util.h"

namespace util {
    // TODO: check the correct endianness depending on the OS
    uint16_t convert_endian(uint16_t data) {
        uint16_t result = 0x0;

        result += ((data >> 8) & 0xFFu);
        result += ((data & 0xFFu) << 8);

        return result;
    }

    uint32_t convert_endian(uint32_t data) {
        uint32_t result = 0x0;

        result += ((data >> 24) & 0xFFul);
        result += ((data & 0xFFul) << 24);
        result += ((data >> 8) & 0xFF00ul);
        result += ((data & 0xFF00ul) << 8);

        return result;
    }

    uint64_t convert_endian(uint64_t data) {
        uint64_t result = 0x0;

        result += ((data >> 56) & 0xFFull);
        result += ((data & 0xFFull) << 56);
        result += ((data >> 40) & 0xFF00ull);
        result += ((data & 0xFF00ull) << 40);
        result += ((data >> 24) & 0xFF0000ull);
        result += ((data & 0xFF0000ull) << 24);
        result += ((data >> 8) & 0xFF000000ull);
        result += ((data & 0xFF000000ull) << 8);

        return result;
    }
}  // namespace util
