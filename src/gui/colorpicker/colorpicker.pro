QT       += core gui

TARGET = colorpicker
TEMPLATE = lib
CONFIG += staticlib

include (../../src_common.pro)

QMAKE_CXXFLAGS += -iquote $$PWD/qt

QMAKE_CXXFLAGS += -iquote $$SOURCE_ROOT/gui/misc
QMAKE_CXXFLAGS += -iquote $$SOURCE_ROOT/support


SOURCES += \
	AlphaSlider.cpp \
	ColorField.cpp \
	ColorPickerPanel.cpp \
	ColorPickerView.cpp \
	ColorPreview.cpp \
	ColorSlider.cpp

HEADERS += \
	AlphaSlider.h \
	ColorField.h \
	ColorPickerPanel.h \
	ColorPickerView.h \
	ColorPreview.h \
	ColorSlider.h \
	SelectedColorMode.h \
	qt/AlphaSliderPlatformDelegate.h \
	qt/ColorFieldPlatformDelegate.h \
	qt/ColorPickerPanelPlatformDelegate.h \
	qt/ColorPickerViewPlatformDelegate.h \
	qt/ColorPreviewPlatformDelegate.h \
	qt/ColorSliderPlatformDelegate.h
