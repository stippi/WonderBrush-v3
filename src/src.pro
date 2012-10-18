QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WonderBrush
TEMPLATE = app

# suppress -Wunused-parameter
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter

SOURCES += \
    WonderBrush_qt.cpp

HEADERS  += \
	WonderBrush.h
