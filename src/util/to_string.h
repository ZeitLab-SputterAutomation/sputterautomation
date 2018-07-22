#pragma once

#include <iostream>
#include <string>
#include <optional>

namespace util {
    namespace detail {
        template <int I>
        struct to_string_choice : to_string_choice<I + 1> {};
        template <>
        struct to_string_choice<4> {};

        // built-in conversion for POD types
        template <class T>
        inline auto to_string_impl(const T &t, to_string_choice<0>)
            -> decltype(std::to_string(t), std::optional<std::string>()) {
            return std::to_string(t);
        }

        // direct "conversion" of narrow-string-like types
        template <class T>
        inline auto to_string_impl(const T &t, to_string_choice<1>)
            -> decltype(std::string(t), std::optional<std::string>()) {
            return std::string(t);
        }

        // using the member function 'std::string to_string() const'
        template <class T>
        inline auto to_string_impl(const T &t, to_string_choice<2>)
            -> decltype(t.to_string(), std::optional<std::string>()) {
            return t.to_string();
        }

        // to_string is non-const
        template <class T>
        inline auto to_string_impl(const T &t, to_string_choice<3>)
            -> decltype(std::declval<typename std::remove_const<T>::type>().to_string(), std::optional<std::string>()) {
            static_assert(sizeof(T) == 0, "Member function to_string() must be const");

            return std::nullopt;
        }

        // no string conversion available
        template <class T>
        inline std::optional<std::string> to_string_impl(const T &t, to_string_choice<4>) {
            static_assert(
                sizeof(T) == 0,
                "The class must either have a public member function 'std::string to_string() const' or an overload for std::to_string must exist");

            return std::nullopt;
        }
    }  // namespace detail

    template <class T>
    inline std::optional<std::string> to_string(const T &t) {
        return detail::to_string_impl(t, detail::to_string_choice<0>{});
    }
}  // namespace util