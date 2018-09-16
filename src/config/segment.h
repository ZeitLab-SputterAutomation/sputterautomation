#pragma once
#include "app_config.h"

#include <memory>
#include <mutex>

#include "util/util.h"

namespace config {
    enum class ValueType { string, number, date };

    template <typename T>
    constexpr ValueType get_type() {
        if constexpr (std::is_arithmetic_v<T>) return ValueType::number;

        /*
        else if constexpr(std::is_same_v<T, DateTime>)
            return ValueType::datetime;
        */

        return ValueType::string;
    }

    class ConfigManager;
    class Segment : public std::enable_shared_from_this<Segment> {
        friend class ConfigManager;

    public:
        Segment() = default;
        Segment(const std::string &name) : m_name(name){};

        Segment(const Segment &) = delete;
        Segment &operator=(const Segment &) = delete;

        std::shared_ptr<Segment> get_segment(const std::string &name, bool create_missing = true);
        std::string serialize() const { return do_serialize(); }

        std::string get_comment() { return m_comment; }
        void set_comment(const std::string &comment) { m_comment = comment; }

        template <typename T>
        std::optional<T> get(const std::string &key) const {
            auto setting = m_settings.find(key);
            if (setting == m_settings.end()) return std::nullopt;

            return util::to_type<T>(setting->second.value);
        }

        template <typename T>
        void set(const std::string &key, const T &value, std::optional<std::string> comment = std::nullopt) {
            auto value_str = util::to_string(value);
            if (!value_str) {
                // TODO: this might fail, since the value couldn't be converted to string in the first place, maybe just
                // warn that a conversion failed?

                m_log->warn("Segment::set(): unable to convert value '{0}' to string", value);
                return;
            }

            // Preserve the old comment if no new one is given
            std::string last_comment = "";
            if (auto it = m_settings.find(key); it != m_settings.end()) last_comment = it->second.comment;

            m_settings[key] = Setting{*value_str, comment.value_or(last_comment), get_type<T>()};
        }

    private:
        // Recursively call do_serialize for all m_children
        std::string do_serialize(const std::string &chain = "", int tablevel = 0) const;

        struct Setting {
            std::string value;
            std::string comment;

            ValueType value_type = ValueType::string;
        };

        std::unordered_map<std::string, std::shared_ptr<Segment>> m_children;
        std::unordered_map<std::string, Setting> m_settings;

        std::string m_comment;
        std::string m_name;

        std::mutex m_mutex;
    };
}  // namespace config
