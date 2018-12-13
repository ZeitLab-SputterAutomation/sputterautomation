#include "ledindicator.h"

#include <memory>
#include <unordered_map>
#include <iostream>
const QPixmap &get_pixmap(LEDIndicator::LEDState state) {
    static std::unordered_map<LEDIndicator::LEDState, QPixmap> pixmaps{
        {LEDIndicator::LEDState::Blue, QPixmap(":/ui/images/led_blue.png")},
        {LEDIndicator::LEDState::Green, QPixmap(":/ui/images/led_green.png")},
        {LEDIndicator::LEDState::DarkGreen, QPixmap(":/ui/images/led_darkgreen.png")},
        {LEDIndicator::LEDState::Red, QPixmap(":/ui/images/led_red.png")},
        {LEDIndicator::LEDState::DarkRed, QPixmap(":/ui/images/led_darkred.png")},
        {LEDIndicator::LEDState::Orange, QPixmap(":/ui/images/led_orange.png")},
        {LEDIndicator::LEDState::Off, QPixmap(":/ui/images/led_off.png")},
        {LEDIndicator::LEDState::UVSymbol, QPixmap(":/ui/images/uvlight_symbol.png")}};

    return pixmaps[state];
}

LEDIndicator::LEDIndicator(QWidget *parent) : QLabel(parent) {
    setScaledContents(true);
    setText(tr(""));
    setPixmap(get_pixmap(LEDState::Off));

    m_blink_timer.setInterval(100);

    connect(&m_blink_timer, &QTimer::timeout, this, &LEDIndicator::handle_timer);
}

void LEDIndicator::set_state(LEDState state) {
    m_current_state = m_selected_state = state;
    m_blink_timer.stop();
    setPixmap(get_pixmap(state));
}

void LEDIndicator::blink(LEDState blink_state) {
    m_selected_state = blink_state;

    m_blink_timer.start();
}

void LEDIndicator::set_blink_interval(int interval) { m_blink_timer.setInterval(interval); }

void LEDIndicator::handle_timer() {
    if (m_current_state == m_selected_state) {
        setPixmap(get_pixmap(LEDState::Off));
        m_current_state = LEDState::Off;
    } else {
        setPixmap(get_pixmap(m_selected_state));
        m_current_state = m_selected_state;
    }
}