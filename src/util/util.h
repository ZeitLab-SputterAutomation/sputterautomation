#pragma once
#include "app_config.h"

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include <QMetaEnum>

#include "base64.h"
#include "util_strings.h"

#if CHAR_BIT != 8
#error CHAR_BIT must be 8 for the endian conversion to work properly
#endif

namespace util {
    uint16_t convert_endian(uint16_t data);
    uint32_t convert_endian(uint32_t data);
    uint64_t convert_endian(uint64_t data);

    // This is an overload for base64_encode found in base64.h/.cpp
    inline std::string base64_encode(std::string_view sv) {
        return ::base64_encode(reinterpret_cast<const unsigned char *>(sv.data()), static_cast<unsigned int>(sv.length()));
    }

    // Convert the value of a Qt-enum to an std::string
    // See https://stackoverflow.com/a/34282031
    template <typename QEnum>
    std::string qt_enum_to_string(const QEnum v) {
        return QMetaEnum::fromType<QEnum>().valueToKey(v);
    }

    template <bool...>
    struct bool_pack;
    template <bool... bs>
    using all_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;

    // are_all_same is used to test if all types in a parameter pack are equal to T
    // See https://stackoverflow.com/a/36934374
    template <typename T, typename... T1s>
    using are_all_same = all_true<std::is_same<T1s, T>::value...>;

    // make_byte_array constructs a QByteArray from its parameters (currently only parameters convertible to uint8_t are possible)
    template <typename... Ts>
    QByteArray make_byte_array(const Ts... args) {
        //static_assert(util::all_true<std::is_convertible_v<Ts, uint8_t>...>::value,
        //              "all parameters in the call to make_byte_array must be convertible to uint8_t");

        QByteArray array;
        array.reserve(sizeof...(args));
        (array.push_back(static_cast<uint8_t>(args)), ...);

        return array;
    }
}  // namespace util
