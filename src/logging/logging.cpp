#include "logging.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace logging {
    void init() {
        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        //sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("log.txt"));

        auto main_logger = std::make_shared<spdlog::logger>("main", std::begin(sinks), std::end(sinks));
        main_logger->set_level(spdlog::level::trace);

        spdlog::register_logger(main_logger);
    }

    log get_log(const std::string &name) { return spdlog::get(name); }
}  // namespace logging
