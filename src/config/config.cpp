#include "config.h"

#include <fstream>

#include "config_manager.h"

namespace config {
    bool load(const std::string &filename, const std::string &identifier) {
        return ConfigManager::instance().load(filename, identifier);
    }

    void load_defaults() {}

    void save(const std::string &filename, const std::string &identifier, bool overwrite = false) {
        auto conf = get_config(identifier);
        if (!conf) {
            logging::get_log("main")->warn("Config::save: unknown config '{0}'", identifier);
            return;
        }

        if (std::filesystem::exists(filename) && !overwrite) {
            logging::get_log("main")->warn("Config::save: config file '{0}' already exists, aborting", filename);
            return;
        }

        std::ofstream file(filename);
        file << conf->serialize();
    }

    std::shared_ptr<ConfigFile> get_config(const std::string &name) { return ConfigManager::instance().get_config(name); }

    // Tries to load the main config file (config.cfg). On failure default values are loaded and saved to the file.
    void init() {
        if (!load("config.cfg", "main")) {
            logging::get_log("main")->info("Main config file (config.cfg) not found, loading default settings");

            load_defaults();
            save("config.cfg", "main");
        }
    }
}  // namespace config
