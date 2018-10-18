#pragma once

#include "spdlog/spdlog.h"

namespace logging {
    using log = std::shared_ptr<spdlog::logger>;

    void init();
    log get_log(const std::string &name);
}  // namespace logging
