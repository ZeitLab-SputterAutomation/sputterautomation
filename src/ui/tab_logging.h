#pragma once

#include <QObject>

class TabLogging : public QObject {
    Q_OBJECT
public:
    TabLogging(QObject *parent = nullptr);
    ~TabLogging() = default;
};