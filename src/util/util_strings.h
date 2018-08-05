#pragma once
#include "app_config.h"

#include <functional>
#include <optional>
#include <sstream>

namespace util {
    // Alternative version of getline to correctly handle lf, cr and crlf
    // See https://stackoverflow.com/a/6089413
    std::istream &safe_getline(std::istream &is, std::string &line);

    // Trims whitespace characters from the beginning of the given string
    void trim_left(std::string::iterator &it, const std::string::iterator &end);
    void trim_left(std::string &s);

    // Trims whitespace characters from the end of the given string
    void trim_right(const std::string::iterator &it, std::string::iterator &end);
    void trim_right(std::string &s);

    // Trims whitespace characters from the beginning and the end of the given string
    void trim_both(std::string::iterator &it, std::string::iterator &end);
    void trim_both(std::string &s);

    // Removes all characters from the given string where pred returns true
    void trim_all(std::string &s, std::function<bool(unsigned char)> pred);

    // Splits a string at a given delimiter. The resulting tokens are saved in the given output container. If
    // allow_empty is false, if an empty token is encountered it will be skipped and split returns false. Otherwise
    // it returns true. If the string does not contain the delimiter, the whole string is returned.
    // Based on https://stackoverflow.com/a/236803
    template <typename Out>
    inline bool split(const std::string &s, char delim, Out result, bool allow_empty = true) {
        std::stringstream ss(s);
        std::string item;
        bool encountered_empty = false;

        while (std::getline(ss, item, delim)) {
            if (item.empty()) {
                encountered_empty = true;

                if (allow_empty) {
                    *(result++) = item;
                }
            } else {
                *(result++) = item;
            }
        }

        return encountered_empty;
    }

    // Splits a string at a given delimiter. The resulting tokens are returned as an optional vector of strings. If
    // allow_empty is false and an empty token is encountered, std::nullopt is returned instead. If the string does
    // not contain the delimiter, the vector contains the string as one element.
    // Based on https://stackoverflow.com/a/236803
    inline std::optional<std::vector<std::string>> split(const std::string &s, char delim, bool allow_empty = true) {
        std::vector<std::string> elems;
        bool encountered_empty = split(s, delim, std::back_inserter(elems), allow_empty);

        if (encountered_empty && !allow_empty) return std::nullopt;

        return elems;
    }
}  // namespace util