TEMPLATE = subdirs

SUBDIRS += \
    src/agg \
    src \
	src/gui/colorpicker \
	src/gui/scrollview \
	src/icon

src.depends = \
	src/agg \
	src/gui/colorpicker \
	src/gui/scrollview \
	src/icon
