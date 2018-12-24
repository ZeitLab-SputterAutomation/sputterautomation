#pragma once
#include "app_config.h"

#include <memory>
#include <mutex>

#include "util/type_conversion.h"
#include "logging/logging.h"

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

        std::shared_ptr<const Segment> get_segment(const std::string &name) const;
        std::shared_ptr<Segment> get_segment(const std::string &name, bool create_missing = true);
        std::string serialize() const { return do_serialize(); }

        std::string get_comment() { return m_comment; }
        void set_comment(const std::string &comment) { m_comment = comment; }

        std::string get_name() { return m_name; }

        template <typename T>
        std::optional<T> get(const std::string &key) const {
            if (key.empty()) {
                logging::get_log("main")->warn("Segment::set(): key is empty");
                return std::nullopt;
            }

            // If a subsegment is meant, load it and get its setting
            if (auto lastpos = key.find_last_of('.'); lastpos != std::string::npos) {
                auto nkey = key.substr(lastpos + 1);
                if (nkey.empty()) {
                    logging::get_log("main")->warn("Segment::set(): last key in chain is empty, got '{0}'", key);
                    return std::nullopt;
                }

                return get_segment(key.substr(0, lastpos))->get<T>(nkey);
            }

            auto setting = m_settings.find(key);
            if (setting == m_settings.end()) return std::nullopt;

            return util::to_type<T>(setting->second.value);
        }

        template <typename T>
        void set(const std::string &key, const T &value, std::optional<std::string> comment = std::nullopt) {
            if (key.empty()) {
                logging::get_log("main")->warn("Segment::set(): key is empty");
                return;
            }

            auto value_str = util::to_string(value);
            if (!value_str) {
                // We cant output the value here since the conversion to string is what failed in the first place
                logging::get_log("main")->warn("Segment::set(): unable to convert value to string for key '{0}'", key);
                return;
            }

            // If a subsegment is meant, load it and set its setting
            if (auto lastpos = key.find_last_of('.'); lastpos != std::string::npos) {
                auto nkey = key.substr(lastpos + 1);
                if (nkey.empty()) {
                    logging::get_log("main")->warn("Segment::set(): last key in chain is empty, got '{0}'", key);
                    return;
                }

                get_segment(key.substr(0, lastpos), true)->set<T>(nkey, value, comment);
                return;
            }

            // Preserve the old comment if no new one is given
            std::string last_comment = "";
            if (auto it = m_settings.find(key); it != m_settings.end()) last_comment = it->second.comment;

            m_settings[key] = Setting{*value_str, comment.value_or(last_comment), get_type<T>()};
        }

        // Returns all settings as a key->value pairs. If subsegment is empty, the settings of this segment are returned,
        // otherwise those of the subsegment (if there is one with the name). All values are returned as strings.
        std::vector<std::pair<std::string, std::string>> get_all(const std::string &subsegment = ""s);

        // Returns all child segments. If subsegment is empty, the children of this segment are returned, otherwise those of the
        // subsegment (if there is one with the name).
        std::vector<std::shared_ptr<Segment>> get_children(const std::string &subsegment = ""s);

    private:
        // Recursively call do_serialize for all m_children
        std::string do_serialize(const std::string &chain = ""s, int tablevel = 0) const;

        struct Setting {
            std::string value;
            std::string comment;

            ValueType value_type = ValueType::string;
        };

        std::unordered_map<std::string, std::shared_ptr<Segment>> m_children;
        std::unordered_map<std::string, Setting> m_settings;

        std::string m_comment;
        std::string m_name;

        mutable std::mutex m_mutex;
    };
}  // namespace config
