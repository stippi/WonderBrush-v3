TEMPLATE = subdirs

SUBDIRS += \
    src/agg \
    src \
    src/gui/scrollview \
	src/icon

src.depends = \
	src/agg \
	src/gui/scrollview \
	src/icon
