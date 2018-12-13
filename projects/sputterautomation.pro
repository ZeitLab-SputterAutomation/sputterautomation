#-------------------------------------------------
#
# Project created by QtCreator 2018-07-22T19:36:42
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = sputterautomation
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++1z

SOURCES += \
    ../src/main.cpp \
    ../src/util/util.cpp \
    ../src/logging/logging.cpp \
    ../src/config/config.cpp \
    ../src/config/config_file.cpp \
    ../src/config/config_manager.cpp \
    ../src/config/segment.cpp \

HEADERS += \
    ../src/app_config.h \
    ../src/stacktrace.h \
    ../src/util/to_string.h \
    ../src/util/type_conversion.h \
    ../src/util/util.h \
    ../src/util/util_strings.h \
    ../src/logging/logging.h \
    ../src/config/config.h \
    ../src/config/config_file.h \
    ../src/config/config_manager.h \
    ../src/config/segment.h \

INCLUDEPATH += $$PWD/../src/

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
