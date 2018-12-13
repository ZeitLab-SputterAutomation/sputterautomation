#pragma once

#include <QObject>

class TabVacuum : public QObject {
    Q_OBJECT
public:
    TabVacuum(QObject *parent = nullptr);
    ~TabVacuum() = default;
};