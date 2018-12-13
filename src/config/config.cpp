#include "config.h"

#include "config_manager.h"

namespace config {
    void init() {}

    bool load(const std::string &file, const std::string &identifier) { return ConfigManager::instance().load(file, identifier); }

    std::shared_ptr<ConfigFile> get_config(const std::string &name) { return ConfigManager::instance().get_config(name); }
}  // namespace config
