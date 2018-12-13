#pragma once

#include <QMainWindow>

#include "ui_mainwindow.h"

#include "config/config.h"

#include "devices/plc/s7.h"
#include "devices/kjl_generator.h"
#include "devices/hofi_switch.h"
#include "devices/cesar_generator.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::mainwindow ui;
};
