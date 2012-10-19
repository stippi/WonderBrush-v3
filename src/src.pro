QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WonderBrush
TEMPLATE = app

INCLUDEPATH += agg/include
INCLUDEPATH += cimg
INCLUDEPATH += platform/qt
INCLUDEPATH += support

DEFINES += __STDC_LIMIT_MACROS=1
DEFINES += __STDC_FORMAT_MACROS=1
DEFINES += _GNU_SOURCE

LIBS += -Lagg -lagg

# suppress -Wunused-parameter
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter

SOURCES += \
	WonderBrush_qt.cpp \
	platform/qt/List.cpp \
	platform/qt/Locker.cpp \
	platform/qt/OS.cpp \
	platform/qt/platform_support.cpp \
	platform/qt/PlatformSemaphoreManager.cpp \
	platform/qt/PlatformThread.cpp \
	platform/qt/Point.cpp \
	platform/qt/Rect.cpp \
	platform/qt/Size.cpp \
	platform/qt/String.cpp \
	support/Command.cpp \
	support/CommandStack.cpp \
	support/CompoundCommand.cpp \
	support/Debug.cpp \
	support/Listener.cpp \
	support/Notifier.cpp \
	support/ObjectTracker.cpp \
	support/Referenceable.cpp \
	support/RWLocker.cpp \
	support/support.cpp \
	support/Transformable.cpp

HEADERS  += \
	cimg/CImg.h \
	platform/qt/List.h \
	platform/qt/Locker.h \
	platform/qt/OS.h \
	platform/qt/platform_support.h \
	platform/qt/PlatformSemaphoreManager.h \
	platform/qt/PlatformThread.h \
	platform/qt/Point.h \
	platform/qt/Rect.h \
	platform/qt/Size.h \
	platform/qt/String.h \
	platform/qt/StringPrivate.h \
	platform/qt/SupportDefs.h \
	platform/qt/utf8_functions.h \
	support/AutoLocker.h \
	support/BuildSupport.h \
	support/Command.h \
	support/CommandStack.h \
	support/CompoundCommand.h \
	support/Debug.h \
	support/DLList.h \
	support/Listener.h \
	support/Notifier.h \
	support/ObjectTracker.h \
	support/Referenceable.h \
	support/RWLocker.h \
	support/support.h \
	support/Transformable.h \
	platform/qt/Errors.h

