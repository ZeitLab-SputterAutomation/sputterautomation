#pragma once

#include <QLabel>
#include <QTimer>

class LEDIndicator : public QLabel {
    Q_OBJECT
public:
    LEDIndicator(QWidget *parent = nullptr);
    ~LEDIndicator() = default;

    enum class LEDState { Off, Blue, Green, Orange, Red, DarkGreen, DarkRed, UVSymbol };

    void set_state(LEDState state);
    LEDState get_state() { return m_current_state; }

    void blink(LEDState blink_state);
    void set_blink_interval(int interval);

private:
    LEDState m_current_state = LEDState::Off;
    LEDState m_selected_state = LEDState::Off;

    QTimer m_blink_timer;

private slots:
    void handle_timer();
};