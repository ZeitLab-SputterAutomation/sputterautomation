#pragma once

#include <memory>

#include <QLabel>
#include <QMovie>

class BusyIndicator : public QLabel {
    Q_OBJECT

public:
    BusyIndicator(QWidget *parent = nullptr);
    ~BusyIndicator() = default;

    enum class BusyState { Inactive, Busy };

    void set_state(BusyState state);
    BusyState getState() { return m_current_state; }

private:
    BusyState m_current_state = BusyState::Inactive;
    std::unique_ptr<QMovie> m_animation;
};