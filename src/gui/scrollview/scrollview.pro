QT       += core gui

TARGET = scrollview
TEMPLATE = lib
CONFIG += staticlib

include (../../src_common.pro)


SOURCES += \
	Scrollable.cpp \
	Scroller.cpp

HEADERS += \
	Scrollable.h \
	Scroller.h
