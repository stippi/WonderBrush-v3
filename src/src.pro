QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WonderBrush
TEMPLATE = app

# TODO: Must be first for now, since otherwise model/objects/Rect.h hides
# platform/qt/Rect.h. This needs to be solved differently, since I believe
# model/objects/Rect.h cannot be included now (though, surprisingly it seems to
# work in model/objects/Rect.cpp).
#INCLUDEPATH += platform/qt
QMAKE_CXXFLAGS += -isystem $$PWD/platform/qt/system/include

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

QMAKE_CXXFLAGS += -iquote $$PWD/commands
QMAKE_CXXFLAGS += -iquote $$PWD/gui/icons
QMAKE_CXXFLAGS += -iquote $$PWD/model
QMAKE_CXXFLAGS += -iquote $$PWD/model/document
QMAKE_CXXFLAGS += -iquote $$PWD/model/fills
QMAKE_CXXFLAGS += -iquote $$PWD/model/objects
QMAKE_CXXFLAGS += -iquote $$PWD/model/property
QMAKE_CXXFLAGS += -iquote $$PWD/model/property/specific_properties
QMAKE_CXXFLAGS += -iquote $$PWD/model/snapshots
QMAKE_CXXFLAGS += -iquote $$PWD/platform/qt
QMAKE_CXXFLAGS += -iquote $$PWD/platform/qt/gui
QMAKE_CXXFLAGS += -iquote $$PWD/platform/qt/system
QMAKE_CXXFLAGS += -iquote $$PWD/render
QMAKE_CXXFLAGS += -iquote $$PWD/support
QMAKE_CXXFLAGS += -iquote $$PWD/tools
QMAKE_CXXFLAGS += -iquote $$PWD/tools/brush
QMAKE_CXXFLAGS += -iquote $$PWD/tools/brush/pick
QMAKE_CXXFLAGS += -iquote $$PWD/tools/brush/transform

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
	WonderBrush.cpp \
	commands/AddObjectsCommand.cpp \
	commands/MoveObjectsCommand.cpp \
	commands/ObjectAddedCommand.cpp \
	commands/SetPropertiesCommand.cpp \
	model/property/CommonPropertyIDs.cpp \
	model/BaseObject.cpp \
	model/Selectable.cpp \
	model/Selection.cpp \
	model/document/Document.cpp \
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
	model/objects/LayerObserver.cpp \
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
	platform/qt/AbstractLOAdapter.cpp \
	platform/qt/platform_bitmap_support.cpp \
	platform/qt/platform_support.cpp \
	platform/qt/platform_support_ui.cpp \
	platform/qt/PlatformSemaphoreManager.cpp \
	platform/qt/PlatformThread.cpp \
	platform/qt/gui/CanvasView.cpp \
	platform/qt/gui/Window.cpp \
	platform/qt/system/ArchivingManagers.cpp \
	platform/qt/system/BAlignment.cpp \
	platform/qt/system/BArchivable.cpp \
	platform/qt/system/BBitmap.cpp \
	platform/qt/system/BByteOrder.cpp \
	platform/qt/system/BDataIO.cpp \
	platform/qt/system/BFile.cpp \
	platform/qt/system/BFlattenable.cpp \
	platform/qt/system/BGraphicsDefs.cpp \
	platform/qt/system/BList.cpp \
	platform/qt/system/BLocker.cpp \
	platform/qt/system/BMessageAdapter.cpp \
	platform/qt/system/BMessage.cpp \
	platform/qt/system/BMessageUtils.cpp \
	platform/qt/system/BMessenger.cpp \
	platform/qt/system/BOS.cpp \
	platform/qt/system/BPoint.cpp \
	platform/qt/system/BPointerList.cpp \
	platform/qt/system/BRect.cpp \
	platform/qt/system/BRegion.cpp \
	platform/qt/system/BRegionSupport.cpp \
	platform/qt/system/BSize.cpp \
	platform/qt/system/BString.cpp \
	render/Font.cpp \
	render/GaussFilter.cpp \
	render/LayoutContext.cpp \
	render/LayoutState.cpp \
	render/Path.cpp \
	render/RenderBuffer.cpp \
	render/RenderEngine.cpp \
	render/RenderManager.cpp \
	render/RenderThread.cpp \
	render/StackBlurFilter.cpp \
	render/TextLayout.cpp \
	render/TextRenderer.cpp \
	render/VertexSource.cpp \
	support/Command.cpp \
	support/CommandStack.cpp \
	support/CompoundCommand.cpp \
	support/Debug.cpp \
	support/Listener.cpp \
	support/ListenerAdapter.cpp \
	support/Notifier.cpp \
	support/ObjectTracker.cpp \
	support/Referenceable.cpp \
	support/RWLocker.cpp \
	support/support.cpp \
	support/Transformable.cpp \
#	tools/DragStateViewState.cpp \
#	tools/Tool.cpp \
#	tools/TransformViewState.cpp \
#	tools/brush/BrushTool.cpp \
#	tools/brush/BrushToolState.cpp \
#	tools/brush/pick/PickTool.cpp \
#	tools/brush/pick/PickToolState.cpp \
#	tools/brush/transform/ChannelTransform.cpp \
#	tools/brush/transform/TransformableGroup.cpp \
#	tools/brush/transform/TransformTool.cpp \
#	tools/brush/transform/TransformToolState.cpp

HEADERS  += \
	cimg/CImg.h \
	commands/AddObjectsCommand.h \
	commands/MoveObjectsCommand.h \
	commands/ObjectAddedCommand.h \
	commands/SetPropertiesCommand.h \
	gui/icons/PathPropertyIcon.h \
	model/BaseObject.h \
	model/Selectable.h \
	model/Selection.h \
	model/document/Document.h \
	model/document/NotifyingList.h \
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
	platform/qt/platform_support_ui.h \
	platform/qt/PlatformSemaphoreManager.h \
	platform/qt/PlatformThread.h \
	platform/qt/PlatformWonderBrush.h \
	platform/qt/gui/CanvasView.h \
	platform/qt/gui/Window.h \
	platform/qt/system/ArchivingManagers.h \
	platform/qt/system/BAlignment.h \
	platform/qt/system/BAppDefs.h \
	platform/qt/system/BArchivable.h \
	platform/qt/system/BAutolock.h \
	platform/qt/system/BBeBuild.h \
	platform/qt/system/BBitmap.h \
	platform/qt/system/BByteOrder.h \
	platform/qt/system/Bclipping.h \
	platform/qt/system/BDataIO.h \
	platform/qt/system/BDebug.h \
	platform/qt/system/Bdebugger.h \
	platform/qt/system/BErrors.h \
	platform/qt/system/BFile.h \
	platform/qt/system/BFlattenable.h \
	platform/qt/system/BGraphicsDefs.h \
	platform/qt/system/Bimage.h \
	platform/qt/system/BInterfaceDefs.h \
	platform/qt/system/BList.h \
	platform/qt/system/BLocker.h \
	platform/qt/system/BMessage.h \
	platform/qt/system/BMessageAdapter.h \
	platform/qt/system/BMessagePrivate.h \
	platform/qt/system/BMessageUtils.h \
	platform/qt/system/BMessenger.h \
	platform/qt/system/BObjectList.h \
	platform/qt/system/BOS.h \
	platform/qt/system/BPoint.h \
	platform/qt/system/BRect.h \
	platform/qt/system/BRegion.h \
	platform/qt/system/BRegionSupport.h \
	platform/qt/system/BSize.h \
	platform/qt/system/BStorageDefs.h \
	platform/qt/system/BString.h \
	platform/qt/system/BStringPrivate.h \
	platform/qt/system/BSupportDefs.h \
	platform/qt/system/BTypeConstants.h \
	platform/qt/system/Butf8_functions.h \
	platform/qt/system/include/Alignment.h \
	platform/qt/system/include/AppDefs.h \
	platform/qt/system/include/Archivable.h \
	platform/qt/system/include/Autolock.h \
	platform/qt/system/include/BeBuild.h \
	platform/qt/system/include/Bitmap.h \
	platform/qt/system/include/ByteOrder.h \
	platform/qt/system/include/clipping.h \
	platform/qt/system/include/DataIO.h \
	platform/qt/system/include/Debug.h \
	platform/qt/system/include/debugger.h \
	platform/qt/system/include/Errors.h \
	platform/qt/system/include/File.h \
	platform/qt/system/include/Flattenable.h \
	platform/qt/system/include/GraphicsDefs.h \
	platform/qt/system/include/image.h \
	platform/qt/system/include/InterfaceDefs.h \
	platform/qt/system/include/List.h \
	platform/qt/system/include/Locker.h \
	platform/qt/system/include/Message.h \
	platform/qt/system/include/MessageAdapter.h \
	platform/qt/system/include/MessagePrivate.h \
	platform/qt/system/include/MessageUtils.h \
	platform/qt/system/include/Messenger.h \
	platform/qt/system/include/ObjectList.h \
	platform/qt/system/include/OS.h \
	platform/qt/system/include/Point.h \
	platform/qt/system/include/Rect.h \
	platform/qt/system/include/Region.h \
	platform/qt/system/include/RegionSupport.h \
	platform/qt/system/include/Size.h \
	platform/qt/system/include/StorageDefs.h \
	platform/qt/system/include/String.h \
	platform/qt/system/include/StringPrivate.h \
	platform/qt/system/include/SupportDefs.h \
	platform/qt/system/include/TypeConstants.h \
	platform/qt/system/include/utf8_functions.h \
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
	support/bitmap_support.h \
	support/BuildSupport.h \
	support/Command.h \
	support/CommandStack.h \
	support/CompoundCommand.h \
	support/Debug.h \
	support/DLList.h \
	support/Listener.h \
	support/ListenerAdapter.h \
	support/Notifier.h \
	support/ObjectCache.h \
	support/ObjectTracker.h \
	support/Referenceable.h \
	support/RWLocker.h \
	support/support.h \
	support/support_ui.h \
	support/Transformable.h \
	support/ui_defines.h \
	tools/DragStateViewState.h \
#	tools/Tool.h \
#	tools/TransformViewState.h \
#	tools/brush/BrushTool.h \
#	tools/brush/BrushToolState.h \
#	tools/brush/pick/PickTool.h \
#	tools/brush/pick/PickToolState.h \
#	tools/brush/transform/ChannelTransform.h \
#	tools/brush/transform/TransformableGroup.h \
#	tools/brush/transform/TransformTool.h \
#	tools/brush/transform/TransformToolState.h

FORMS += \
    platform/qt/gui/Window.ui
