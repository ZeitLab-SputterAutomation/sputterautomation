#pragma once

#include <QObject>

class TabTargets : public QObject {
    Q_OBJECT
public:
    TabTargets(QObject *parent = nullptr);
    ~TabTargets() = default;
};