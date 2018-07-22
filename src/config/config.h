#pragma once

#include "config_file.h"

namespace config {
    void init();

    bool load(const std::string &file, const std::string &identifier);
    std::shared_ptr<ConfigFile> get_config(const std::string &name);
}  // namespace config
