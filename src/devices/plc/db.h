#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace PLC {
    // DB: Data Block
    template <typename T>
    struct DB {
        T data;

        std::string name;
        int address_db;
        int address_data;
    };

    using DBword = DB<uint16_t>;
    using DBdword = DB<uint32_t>;

    // DBVector: Data Block with variable length
    // This holds the data of multiple data blocks.
    template <typename T>
    struct DBVector {
        std::vector<T> data;

        int length;
        int address;
    };

    using DBDataword = DBVector<uint16_t>;
    using DBDatadword = DBVector<uint32_t>;
}  // namespace PLC