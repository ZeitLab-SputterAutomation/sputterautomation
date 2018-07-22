#include <iostream>

#include "config/config.h"
#include "logging/logging.h"
#include "stacktrace.h"

int main(int argc, char *argv[]) {
    try {
        logging::init();
        config::init();

        // load Qt app
    } catch (const std::exception &e) {
        std::string msg{"Exception: "};
        msg += e.what();

        const boost::stacktrace::stacktrace *st = boost::get_error_info<traced>(e);
        if (st) {
            std::stringstream ss;
            ss << *st;
            msg += "\nStacktrace:\n" + ss.str();
        }
        msg += '\n';

        spdlog::get("main")->critical(msg);
    }

    std::cin.get();
    return 0;
}
