#pragma once

#include <sstream>
#include <type_traits>
#include <optional>

#include "to_string.h"

namespace util {
    namespace detail {
        // (Quirin) Note 23/09/2018:
        // Conformance mode (/permissive-) causes std::underlying_type_t, although wrapped inside an if constexpr to
        // check for enum, to generate compilation errors.

        // clang-format off
        template <typename Out, typename In>
        struct enable_conversion
            : std::bool_constant<std::is_constructible_v<std::string, Out>
                              || std::is_arithmetic_v<Out>
                              || std::is_enum_v<Out>
                              || std::is_enum_v<In>> {};
        // clang-format on

        // string- or arithmetic types
        template <typename Out, typename In>
        inline auto to_type_impl(const In &t) ->
            typename std::enable_if_t<enable_conversion<Out, In>::value, std::optional<Out>> {
            // for conversions to and from enums we use static_cast
            if constexpr (std::is_enum_v<In>) {
                // first convert the enum value to its underlying type
                using utype = std::underlying_type_t<In>;
                auto u = static_cast<utype>(t);

                // then convert it to the desired Out-type
                return util::to_type<Out>(u);
            } else if constexpr (std::is_enum_v<Out>) {
                // first convert to the underlying type of the enum
                using utype = std::underlying_type_t<Out>;
                auto u = util::to_type<utype>(t);

                if (!u) return std::nullopt;

                // then convert to the enum-type
                return static_cast<Out>(*u);
            }

            // use util::to_string for conversions to string to enable serialization of classes via to_string member-
            // function
            else if constexpr (std::is_same_v<Out, std::string>)
                return to_string(t);

            // for conversions from strings we use the built-in methods which also do some checks
            else if constexpr (std::is_constructible_v<std::string, In>) {
                std::string in{t};

                // catch any exceptions and interpret them as no conversion possible
                try {
                    if constexpr (std::is_same_v<Out, int>) return std::stoi(in, nullptr, 0);
                    if constexpr (std::is_same_v<Out, long>) return std::stol(in, nullptr, 0);
                    if constexpr (std::is_same_v<Out, float>) return std::stof(in, nullptr);
                    if constexpr (std::is_same_v<Out, double>) return std::stod(in, nullptr);
                } catch (...) {
                    return std::nullopt;
                }
            }

            // else use a stringstream
            else {
                Out out{};

                std::stringstream ss;
                ss << t;
                ss >> out;

                if (ss.fail()) return std::nullopt;

                return out;
            }

            // To fix the warning "C4715: not all control paths return a value"
            return std::nullopt;
        }

        // no conversion possible for types other that string-, arithmetic- and enum-types
        template <typename Out, typename In, typename = typename std::enable_if_t<!enable_conversion<Out, In>::value, void>>
        inline std::optional<Out> to_type_impl(const In &t) {
            static_assert(sizeof(Out) == 0, "Only conversion to and from POD-, enum- and string-types possible");

            return std::nullopt;
        }
    }  // namespace detail

    // Converts between narrow string types, POD types and enum (class)
    template <typename Out, typename In>
    inline std::optional<Out> to_type(const In &t) {
        return detail::to_type_impl<Out, In>(t);
    }
}  // namespace util
