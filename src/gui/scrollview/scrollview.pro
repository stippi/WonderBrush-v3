QT       += core gui

TARGET = scrollview
TEMPLATE = lib
CONFIG += staticlib

include (../../src_common.pro)

QMAKE_CXXFLAGS += -iquote $$PWD/qt


SOURCES += \
	Scrollable.cpp \
	Scroller.cpp \
	qt/ScrollView.cpp

HEADERS += \
	Scrollable.h \
	Scroller.h \
	qt/ScrollView.h
