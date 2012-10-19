QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WonderBrush
TEMPLATE = app

INCLUDEPATH += agg/include
INCLUDEPATH += cimg
INCLUDEPATH += model
INCLUDEPATH += model/property
INCLUDEPATH += model/property/specific_properties
INCLUDEPATH += platform/qt
INCLUDEPATH += support

DEFINES += __STDC_LIMIT_MACROS=1
DEFINES += __STDC_FORMAT_MACROS=1
DEFINES += _GNU_SOURCE

LIBS += -Lagg -lagg -ldl

# suppress undesired warnings
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-multichar

SOURCES += \
	WonderBrush_qt.cpp \
	model/property/CommonPropertyIDs.cpp \
	model/BaseObject.cpp \
	model/Selectable.cpp \
	model/Selection.cpp \
	model/property/Property.cpp \
	model/property/PropertyObject.cpp \
	model/property/PropertyObjectProperty.cpp \
	model/property/specific_properties/ColorProperty.cpp \
	model/property/specific_properties/IconProperty.cpp \
	model/property/specific_properties/Int64Property.cpp \
	model/property/specific_properties/OptionProperty.cpp \
	platform/qt/Alignment.cpp \
	platform/qt/Archivable.cpp \
	platform/qt/ArchivingManagers.cpp \
	platform/qt/BRect.cpp \
	platform/qt/ByteOrder.cpp \
	platform/qt/DataIO.cpp \
	platform/qt/Flattenable.cpp \
	platform/qt/List.cpp \
	platform/qt/Locker.cpp \
	platform/qt/Message.cpp \
	platform/qt/MessageAdapter.cpp \
	platform/qt/MessageUtils.cpp \
	platform/qt/OS.cpp \
	platform/qt/platform_support.cpp \
	platform/qt/PlatformSemaphoreManager.cpp \
	platform/qt/PlatformThread.cpp \
	platform/qt/Point.cpp \
	platform/qt/PointerList.cpp \
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
	model/BaseObject.h \
	model/Selectable.h \
	model/Selection.h \
	model/property/CommonPropertyIDs.h \
	model/property/Property.h \
	model/property/PropertyObject.h \
	model/property/PropertyObjectProperty.h \
	model/property/specific_properties/ColorProperty.h \
	model/property/specific_properties/IconProperty.h \
	model/property/specific_properties/Int64Property.h \
	model/property/specific_properties/OptionProperty.h \
	platform/qt/Alignment.h \
	platform/qt/Archivable.h \
	platform/qt/ArchivingManagers.h \
	platform/qt/AppDefs.h \
	platform/qt/BeBuild.h \
	platform/qt/ByteOrder.h \
	platform/qt/DataIO.h \
	platform/qt/Errors.h \
	platform/qt/Flattenable.h \
	platform/qt/GraphicsDefs.h \
	platform/qt/image.h \
	platform/qt/InterfaceDefs.h \
	platform/qt/List.h \
	platform/qt/Locker.h \
	platform/qt/Message.h \
	platform/qt/MessageAdapter.h \
	platform/qt/MessagePrivate.h \
	platform/qt/MessageUtils.h \
	platform/qt/ObjectList.h \
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
	platform/qt/TypeConstants.h \
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
	support/support_ui.h \
	support/Transformable.h \
	support/ui_defines.h

