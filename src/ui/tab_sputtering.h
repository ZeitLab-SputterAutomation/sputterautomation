#pragma once

#include <QObject>

class TabSputtering : public QObject {
    Q_OBJECT
public:
    TabSputtering(QObject *parent = nullptr);
    ~TabSputtering() = default;
};