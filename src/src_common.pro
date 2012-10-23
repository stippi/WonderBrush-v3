SOURCE_ROOT=$$PWD
OUTPUT_ROOT=$$OUT_PWD

# TODO: Must be first for now, since otherwise model/objects/Rect.h hides
# platform/qt/Rect.h. This needs to be solved differently, since I believe
# model/objects/Rect.h cannot be included now (though, surprisingly it seems to
# work in model/objects/Rect.cpp).
#INCLUDEPATH += platform/qt
QMAKE_CXXFLAGS += -isystem $$SOURCE_ROOT/platform/qt/system/include

QMAKE_CXXFLAGS += -iquote $$SOURCE_ROOT/platform/qt
QMAKE_CXXFLAGS += -iquote $$SOURCE_ROOT/platform/qt/system

DEFINES += __STDC_LIMIT_MACROS=1
DEFINES += __STDC_FORMAT_MACROS=1
DEFINES += _GNU_SOURCE

# suppress undesired warnings
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-multichar
