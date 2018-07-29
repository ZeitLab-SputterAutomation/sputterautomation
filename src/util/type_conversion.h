#pragma once

#include <sstream>
#include <optional>

#include "to_string.h"

namespace util {
    namespace detail {
        // conversion to string
        template <typename Out, typename In,
                  typename = typename std::enable_if<std::is_same<Out, std::string>::value>::type>
        inline std::optional<std::string> to_type_impl(const In &t) {
            return to_string(t);
        }

        // conversion using a stringstream
        template <typename Out, typename In, typename = typename std::enable_if<std::is_arithmetic<Out>::value>::type>
        inline std::optional<Out> to_type_impl(const In &t) {
            std::stringstream ss;
            ss << t;
            Out o{};
            ss >> o;
            return o;
        }

        // no conversion possible for these types
        template <typename Out, typename In>
        inline auto to_type_impl(const In &t) ->
            typename std::enable_if<!std::is_arithmetic<Out>::value && !std::is_same<Out, std::string>::value,
                                    std::optional<Out>>::type {
            static_assert(sizeof(Out) == 0, "Only POD- and string-types allowed");

            return std::nullopt;
        }
    }  // namespace detail

    template <typename Out, typename In>
    inline std::optional<Out> to_type(const In &t) {
        return detail::to_type_impl<Out, In>(t);
    }
}  // namespace util