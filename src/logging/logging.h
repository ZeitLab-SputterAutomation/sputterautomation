#pragma once

#include "spdlog/spdlog.h"

// Currently formatting in log messages using fmt's way (ie. {}) causes an internal compiler error in the project, so
// use string concatenation and the type conversion functions in type_conversion.h

namespace logging {
    using log = std::shared_ptr<spdlog::logger>;

    void init();
    log get_log(const std::string &name);
}  // namespace logging
