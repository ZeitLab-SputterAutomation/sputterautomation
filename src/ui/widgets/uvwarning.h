#pragma once

#include <QLabel>
#include <QTimer>

class UVWarning : public QLabel {
    Q_OBJECT
public:
    UVWarning(QWidget *parent = nullptr);
    ~UVWarning() = default;

    void blink(int period);

    bool is_blinking();

private:
    bool m_shown = false;

    QTimer m_blink_timer;

private slots:
    void handle_timer();
};