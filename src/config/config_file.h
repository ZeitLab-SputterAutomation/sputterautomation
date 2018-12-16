#pragma once
#include "app_config.h"

#include <filesystem>
#include <optional>

#include "logging/logging.h"
#include "segment.h"

// TODO: we don't need ConfigFile anymore, replace it directly with Segment
namespace config {
    class ConfigManager;
    class ConfigFile {
        friend class ConfigManager;

    public:
        ConfigFile() noexcept : m_root{std::make_shared<Segment>()}, m_log{logging::get_log("main")} {}
        ConfigFile(const std::string &path) noexcept : ConfigFile::ConfigFile() { m_path = path; }

        ConfigFile(const ConfigFile &) = delete;
        ConfigFile &operator=(const ConfigFile &) = delete;

        ~ConfigFile() {
            // write out config file
            save();
        }

        template <typename T>
        std::optional<T> get(const std::string &key) const {
            // If the setting is located in a subsegment, load it first
            if (auto dotpos = key.find_last_of('.'); dotpos != std::string::npos) {
                auto seg = m_root->get_segment(key.substr(0, dotpos), false);

                if (!seg) {
                    m_log->warn("ConfigFile::get(): encountered empty segment name in key '{0}'", key);

                    return std::nullopt;
                }

                return seg->get<T>(key.substr(dotpos + 1));
            }

            return m_root->get<T>(key);
        };

        template <typename T>
        void set(const std::string &key, const T &value, std::optional<std::string> comment = std::nullopt) {
            // If the setting is located in a subsegment, load it first
            if (auto dotpos = key.find_last_of('.'); dotpos != std::string::npos) {
                auto seg = m_root->get_segment(key.substr(0, dotpos), true);

                if (!seg) {
                    m_log->warn("ConfigFile::set(): encountered empty segment name in key '{0}'", key);

                    return;
                }

                seg->set<T>(key.substr(dotpos + 1), value, comment);
            } else {
                m_root->set<T>(key, value, comment);
            }
            m_changed = true;
        }

        std::string serialize() const {
            if (m_root) return m_root->serialize();

            return "";
        };

        void set_comment(const std::string &comment) { m_comment = comment; };

        void save();

        // returns the underlying segment
        std::shared_ptr<Segment> get_segment(const std::string &name = ""s) { 
            return m_root->get_segment(name);
        }

    private:
        std::filesystem::path m_path;
        std::string m_name;
        std::string m_comment{""};

        bool m_changed = false;

        std::shared_ptr<Segment> m_root;

        logging::log m_log;
    };
}  // namespace config
