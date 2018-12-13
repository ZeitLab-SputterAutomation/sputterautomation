#pragma once

#include <QObject>

class TabLock : public QObject {
    Q_OBJECT
public:
    TabLock(QObject *parent = nullptr);
    ~TabLock() = default;
};