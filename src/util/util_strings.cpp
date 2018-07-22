#include "util_strings.h"

#include <algorithm>
#include <cctype>

namespace util {
    std::istream &safe_getline(std::istream &is, std::string &line) {
        line.clear();

        std::istream::sentry sentry{is, true};
        auto sb = is.rdbuf();

        while (true) {
            auto c = sb->sbumpc();

            switch (c) {
            case '\n':
                return is;
            case '\r':
                if (sb->sgetc() == '\n') sb->sbumpc();
                return is;
            case std::streambuf::traits_type::eof():
                // Also handle the case when the last line has no line ending
                if (line.empty()) is.setstate(std::ios::eofbit);
                return is;
            default:
                line.push_back(static_cast<char>(c));
            }
        }
    }

    void trim_left(std::string::iterator &it, const std::string::iterator &end) {
        it = std::find_if(it, end, [](unsigned char c) { return !std::isspace(c); });
    }

    void trim_left(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); }));
    }

    void trim_right(const std::string::iterator &it, std::string::iterator &end) {
        if (it > end) return;

        // Crude implementation of a "reverse" find_if (based on https://en.cppreference.com/w/cpp/algorithm/find second
        // version)
        while (it != end) {
            end = std::prev(end);

            if (!isspace(static_cast<unsigned char>(*end))) {
                end = std::next(end);
                return;
            }
        }
    }

    void trim_right(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
    }

    void trim_both(std::string::iterator &it, std::string::iterator &end) {
        trim_left(it, end);
        trim_right(it, end);
    }

    void trim_both(std::string &s) {
        trim_left(s);
        trim_right(s);
    }

    void trim_all(std::string &s, std::function<bool(unsigned char)> pred) {
        s.erase(std::remove_if(s.begin(), s.end(), pred), s.end());
    }
}  // namespace util
