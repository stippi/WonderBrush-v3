QT       += core gui

TARGET = icon
TEMPLATE = lib
CONFIG += staticlib

include (../src_common.pro)

INCLUDEPATH += $$SOURCE_ROOT/agg/include

QMAKE_CXXFLAGS += -isystem $$PWD/include

QMAKE_CXXFLAGS += -iquote $$PWD/flat_icon
QMAKE_CXXFLAGS += -iquote $$PWD/message
QMAKE_CXXFLAGS += -iquote $$PWD/shape
QMAKE_CXXFLAGS += -iquote $$PWD/style
QMAKE_CXXFLAGS += -iquote $$PWD/transformable
QMAKE_CXXFLAGS += -iquote $$PWD/transformer


SOURCES += \
	Icon.cpp \
	IconRenderer.cpp \
	IconUtils.cpp \
	flat_icon/FlatIconFormat.cpp \
	flat_icon/FlatIconImporter.cpp \
	flat_icon/LittleEndianBuffer.cpp \
	flat_icon/PathCommandQueue.cpp \
	message/Defines.cpp \
	message/MessageImporter.cpp \
	shape/PathContainer.cpp \
	shape/ShapeContainer.cpp \
	shape/Shape.cpp \
	shape/VectorPath.cpp \
	style/GradientTransformable.cpp \
	style/StyleContainer.cpp \
	style/Style.cpp \
	transformable/Transformable.cpp \
	transformer/AffineTransformer.cpp \
	transformer/ContourTransformer.cpp \
	transformer/PathSource.cpp \
	transformer/PerspectiveTransformer.cpp \
	transformer/StrokeTransformer.cpp \
	transformer/Transformer.cpp \
	transformer/TransformerFactory.cpp


HEADERS += \
	Icon.h \
	IconBuild.h \
	IconRenderer.h \
	flat_icon/FlatIconFormat.h \
	flat_icon/FlatIconImporter.h \
	flat_icon/LittleEndianBuffer.h \
	flat_icon/PathCommandQueue.h \
	include/IconUtils.h \
	message/Defines.h \
	message/MessageImporter.h \
	shape/PathContainer.h \
	shape/ShapeContainer.h \
	shape/Shape.h \
	shape/VectorPath.h \
	style/GradientTransformable.h \
	style/StyleContainer.h \
	style/Style.h \
	transformable/Transformable.h \
	transformer/AffineTransformer.h \
	transformer/ContourTransformer.h \
	transformer/PathSource.h \
	transformer/PerspectiveTransformer.h \
	transformer/StrokeTransformer.h \
	transformer/TransformerFactory.h \
	transformer/Transformer.h
