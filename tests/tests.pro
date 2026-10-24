QT += core gui widgets multimedia

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = spawn_rules_test

GAME = $$PWD/..
INCLUDEPATH += $$GAME

SOURCES += \
    spawn_rules_test.cpp \
    $$files($$GAME/*.cpp)
SOURCES -= $$GAME/main.cpp

HEADERS += \
    $$files($$GAME/*.h)
