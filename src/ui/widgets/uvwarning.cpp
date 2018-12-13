#include "uvwarning.h"

UVWarning::UVWarning(QWidget *parent) : QLabel(parent) {
    setScaledContents(true);
    setText(tr(""));
    setPixmap(QPixmap(":/ui/images/uvlight_symbol.png"));
    setVisible(false);

    connect(&m_blink_timer, &QTimer::timeout, this, &UVWarning::handle_timer);
}

void UVWarning::blink(int period) {
    if (period == 0) {
        m_blink_timer.stop();
        setVisible(false);
        m_shown = false;
    } else {
        m_blink_timer.start(period);
    }
}

bool UVWarning::is_blinking() { return m_blink_timer.isActive(); }

void UVWarning::handle_timer() {
    m_shown = !m_shown;
    setVisible(m_shown);
}