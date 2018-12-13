#pragma once

#include <QObject>

class TabSputterrecipe : public QObject {
    Q_OBJECT
public:
    TabSputterrecipe(QObject *parent = nullptr);
    ~TabSputterrecipe() = default;
};