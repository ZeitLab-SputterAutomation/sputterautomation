#include "busyindicator.h"

BusyIndicator::BusyIndicator(QWidget *parent) : QLabel(parent) {
    setScaledContents(true);
    setText(tr(""));

    m_animation = std::make_unique<QMovie>(":/ui/images/busy_indicator.gif");
    setMovie(m_animation.get());

    setVisible(true);
    m_animation->start();
}

void BusyIndicator::set_state(BusyState state) {
    m_current_state = state;

    if (state == BusyState::Inactive) {
        m_animation->stop();
        setVisible(false);
    } else {
        m_animation->start();
        setVisible(true);
    }
}