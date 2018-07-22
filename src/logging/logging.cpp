#include "logging.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace logging {
    void init() {
        auto main_logger = spdlog::stdout_color_mt("main");
        main_logger->set_level(spdlog::level::trace);
    }

    log get_log(const std::string &name) {
        return spdlog::get(name);
    }
}  // namespace logging
