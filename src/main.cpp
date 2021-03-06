#include <iostream>

#include <QApplication>

#include "config/config.h"
#include "logging/logging.h"
#include "mainwindow.h"
#include "stacktrace.h"

int main(int argc, char *argv[]) {
    try {
        logging::init();
        config::init();

        QApplication app(argc, argv);
        MainWindow window;

        window.show();

        // If cleanup code is needed, connect to the QCoreApplication::aboutToQuit signal

        return app.exec();

    } catch (const std::exception &e) {
        std::string msg{"Unhandled exception: "};
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
