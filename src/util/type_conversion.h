#pragma once

#include <sstream>
#include <type_traits>
#include <optional>

#include <boost/lexical_cast.hpp>

#include "to_string.h"

namespace util {
    namespace detail {
        // string- or arithmetic types
        template <typename Out, typename In>
        inline auto to_type_impl(const In &t) ->
            typename std::enable_if_t<std::is_constructible_v<std::string, Out> || std::is_arithmetic_v<Out>,
                                      std::optional<Out>> {
            // use util::to_string for conversions to string to enable serialization of classes via to_string member-
            // function
            if constexpr (std::is_same_v<Out, std::string>) return to_string(t);

            // for conversions from strings we use the built-in methods which also do some checks
            if constexpr (std::is_constructible_v<std::string, In>) {
                std::string in{t};

                // catch any exceptions and interpret them as no conversion possible
                try {
                    if constexpr (std::is_same_v<Out, int>) return std::stoi(in, 0, 0);
                    if constexpr (std::is_same_v<Out, long>) return std::stol(in, 0, 0);
                    if constexpr (std::is_same_v<Out, float>) return std::stof(in, 0);
                    if constexpr (std::is_same_v<Out, double>) return std::stod(in, 0);
                } catch (...) {
                    return std::nullopt;
                }
            }

            // else use lexical cast
            Out out{};
            if (!boost::conversion::try_lexical_convert(t, out)) return std::nullopt;

            return out;
        }

        // no conversion possible non-string and -arithmetic types
        template <typename Out, typename In,
                  typename = typename std::enable_if_t<
                      !std::is_constructible_v<std::string, Out> && !std::is_arithmetic_v<Out>, void>>
        inline std::optional<Out> to_type_impl(const In &t) {
            static_assert(sizeof(Out) == 0, "Only conversion to POD- and string-types allowed");

            return std::nullopt;
        }
    }  // namespace detail

    template <typename Out, typename In>
    inline std::optional<Out> to_type(const In &t) {
        return detail::to_type_impl<Out, In>(t);
    }
}  // namespace util
