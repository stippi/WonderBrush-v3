TEMPLATE = subdirs

SUBDIRS += \
    src/agg \
    src \
    src/gui/scrollview

src.depends = \
	src/agg \
	src/gui/scrollview
