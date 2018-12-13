#pragma once

#include <QObject>

class Watchtower : public QObject {
    Q_OBJECT
private:
    Watchtower() = default;
    Watchtower(const Watchtower &) = delete;
    Watchtower &operator=(const Watchtower &) = delete;

public:
    ~Watchtower() = default;

    // Thread-safe as of C++11 (§6.7 [stmt.dcl] p4)
    static Watchtower &instance() noexcept {
        static Watchtower wt;
        return wt;
    }

    void load_devices(const std::string &file);

private:


};