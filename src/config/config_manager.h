#pragma once
#include "app_config.h"

#include "config_file.h"

#include "logging/logging.h"

namespace config {
    class ConfigManager {
    private:
        ConfigManager() noexcept : m_log{logging::get_log("main")} {}

        ConfigManager(const ConfigManager &) = delete;
        ConfigManager &operator=(const ConfigManager &) = delete;

    public:
        ~ConfigManager() { close_all(); };

        // Thread-safe as of C++11 (§6.7 [stmt.dcl] p4)
        static ConfigManager &instance() noexcept {
            static ConfigManager manager;
            return manager;
        }

        // Returns nullptr if config "name" is unavailable (and logs it)
        std::shared_ptr<ConfigFile> get_config(const std::string &name);

        void close_all();

        // TODO: implement
        //void create_config();
        //void save_config();

        bool load(const std::string &file, const std::string &identifier);

    private:
        bool parse(std::istream &stream, std::shared_ptr<ConfigFile> conf);

        std::unordered_map<std::string, std::shared_ptr<ConfigFile>> m_configs;

        std::mutex m_mutex;

        logging::log m_log;
    };
}  // namespace config