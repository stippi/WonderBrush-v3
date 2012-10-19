QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WonderBrush
TEMPLATE = app

# TODO: Must be first for now, since otherwise model/objects/Rect.h hides
# platform/qt/Rect.h. This needs to be solved differently, since I believe
# model/objects/Rect.h cannot be included now (though, surprisingly it seems to
# work in model/objects/Rect.cpp).
#INCLUDEPATH += platform/qt
QMAKE_CXXFLAGS += -isystem $$PWD/platform/qt

INCLUDEPATH += /usr/include/freetype2
INCLUDEPATH += agg/font_freetype
INCLUDEPATH += agg/include
INCLUDEPATH += cimg

#INCLUDEPATH += model
#INCLUDEPATH += model/fills
#INCLUDEPATH += model/objects
#INCLUDEPATH += model/property
#INCLUDEPATH += model/property/specific_properties
#INCLUDEPATH += model/snapshots
#INCLUDEPATH += render
#INCLUDEPATH += support

QMAKE_CXXFLAGS += -iquote $$PWD/gui/icons
QMAKE_CXXFLAGS += -iquote $$PWD/model
QMAKE_CXXFLAGS += -iquote $$PWD/model/fills
QMAKE_CXXFLAGS += -iquote $$PWD/model/objects
QMAKE_CXXFLAGS += -iquote $$PWD/model/property
QMAKE_CXXFLAGS += -iquote $$PWD/model/property/specific_properties
QMAKE_CXXFLAGS += -iquote $$PWD/model/snapshots
QMAKE_CXXFLAGS += -iquote $$PWD/render
QMAKE_CXXFLAGS += -iquote $$PWD/support

DEFINES += __STDC_LIMIT_MACROS=1
DEFINES += __STDC_FORMAT_MACROS=1
DEFINES += _GNU_SOURCE

LIBS += -Lagg -lagg -ldl -lfreetype

# Weirdly we need to explicitly add libX11, since otherwise the linker complains
# about symbol XGetWindowAttributes not being defined.
LIBS += -lX11

# suppress undesired warnings
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-multichar

SOURCES += \
	WonderBrush_qt.cpp \
	model/property/CommonPropertyIDs.cpp \
	model/BaseObject.cpp \
	model/Selectable.cpp \
	model/Selection.cpp \
	model/fills/Brush.cpp \
	model/fills/Color.cpp \
	model/fills/Gradient.cpp \
	model/fills/Paint.cpp \
	model/fills/StrokeProperties.cpp \
	model/fills/Style.cpp \
	model/objects/BoundedObject.cpp \
	model/objects/BrushStroke.cpp \
	model/objects/Filter.cpp \
	model/objects/Image.cpp \
	model/objects/Layer.cpp \
#	model/objects/LayerObserver.cpp \
	model/objects/Object.cpp \
	model/objects/Rect.cpp \
	model/objects/Shape.cpp \
	model/objects/Styleable.cpp \
	model/property/Property.cpp \
	model/property/PropertyObject.cpp \
	model/property/PropertyObjectProperty.cpp \
	model/property/specific_properties/ColorProperty.cpp \
	model/property/specific_properties/IconProperty.cpp \
	model/property/specific_properties/Int64Property.cpp \
	model/property/specific_properties/OptionProperty.cpp \
	model/snapshots/BrushStrokeSnapshot.cpp \
	model/snapshots/FilterSnapshot.cpp \
	model/snapshots/ImageSnapshot.cpp \
	model/snapshots/LayerSnapshot.cpp \
	model/snapshots/ObjectSnapshot.cpp \
	model/snapshots/RectSnapshot.cpp \
	model/snapshots/ShapeSnapshot.cpp \
	model/snapshots/StyleableSnapshot.cpp \
	platform/qt/Alignment.cpp \
	platform/qt/Archivable.cpp \
	platform/qt/ArchivingManagers.cpp \
	platform/qt/BBitmap.cpp \
	platform/qt/BRect.cpp \
	platform/qt/BRegion.cpp \
	platform/qt/BRegionSupport.cpp \
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
	render/Font.cpp \
	render/GaussFilter.cpp \
	render/LayoutContext.cpp \
	render/LayoutState.cpp \
	render/Path.cpp \
	render/RenderBuffer.cpp \
	render/RenderEngine.cpp \
#	render/RenderManager.cpp \
#	render/RenderThread.cpp \
	render/StackBlurFilter.cpp \
	render/TextLayout.cpp \
	render/TextRenderer.cpp \
	render/VertexSource.cpp \
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
	gui/icons/PathPropertyIcon.h \
	model/BaseObject.h \
	model/Selectable.h \
	model/Selection.h \
	model/fills/BlendingMode.h \
	model/fills/Brush.h \
	model/fills/Color.h \
	model/fills/Gradient.h \
	model/fills/Paint.h \
	model/fills/SetProperty.h \
	model/fills/SharedObjectCache.h \
	model/fills/StrokeProperties.h \
	model/fills/Style.h \
	model/objects/BoundedObject.h \
	model/objects/BrushStroke.h \
	model/objects/Filter.h \
	model/objects/Image.h \
	model/objects/Layer.h \
	model/objects/LayerObserver.h \
	model/objects/Object.h \
	model/objects/Rect.h \
	model/objects/Shape.h \
	model/objects/Styleable.h \
	model/property/CommonPropertyIDs.h \
	model/property/Property.h \
	model/property/PropertyObject.h \
	model/property/PropertyObjectProperty.h \
	model/property/specific_properties/ColorProperty.h \
	model/property/specific_properties/IconProperty.h \
	model/property/specific_properties/Int64Property.h \
	model/property/specific_properties/OptionProperty.h \
	model/snapshots/BrushStrokeSnapshot.h \
	model/snapshots/FilterSnapshot.h \
	model/snapshots/ImageSnapshot.h \
	model/snapshots/LayerSnapshot.h \
	model/snapshots/ObjectSnapshot.h \
	model/snapshots/RectSnapshot.h \
	model/snapshots/ShapeSnapshot.h \
	model/snapshots/StyleableSnapshot.h \
	platform/qt/Alignment.h \
	platform/qt/Archivable.h \
	platform/qt/ArchivingManagers.h \
	platform/qt/AppDefs.h \
	platform/qt/BeBuild.h \
	platform/qt/Bitmap.h \
	platform/qt/ByteOrder.h \
	platform/qt/clipping.h \
	platform/qt/DataIO.h \
	platform/qt/Debug.h \
	platform/qt/debugger.h \
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
	platform/qt/Region.h \
	platform/qt/RegionSupport.h \
	platform/qt/Size.h \
	platform/qt/String.h \
	platform/qt/StringPrivate.h \
	platform/qt/SupportDefs.h \
	platform/qt/TypeConstants.h \
	platform/qt/utf8_functions.h \
	render/FauxWeight.h \
	render/Font.h \
	render/GaussFilter.h \
	render/LayoutContext.h \
	render/LayoutState.h \
	render/Path.h \
	render/RenderBuffer.h \
	render/RenderEngine.h \
	render/RenderManager.h \
	render/RenderThread.h \
	render/Scanline.h \
	render/StackBlurFilter.h \
	render/TextLayout.h \
	render/TextRenderer.h \
	render/VertexSource.h \
	support/AbstractLOAdapter.h \
	support/AutoLocker.h \
	support/BuildSupport.h \
	support/Command.h \
	support/CommandStack.h \
	support/CompoundCommand.h \
	support/Debug.h \
	support/DLList.h \
	support/Listener.h \
	support/Notifier.h \
	support/ObjectCache.h \
	support/ObjectTracker.h \
	support/Referenceable.h \
	support/RWLocker.h \
	support/support.h \
	support/support_ui.h \
	support/Transformable.h \
	support/ui_defines.h
