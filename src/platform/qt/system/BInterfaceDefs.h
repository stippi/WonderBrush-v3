/*
 * Copyright 2001-2008, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef INTERFACE_DEFS_H
#define INTERFACE_DEFS_H


#include <GraphicsDefs.h>
#include <OS.h>


class BBitmap;
class BPoint;
class BRect;


enum alignment {
	B_ALIGN_LEFT,
	B_ALIGN_RIGHT,
	B_ALIGN_CENTER,

	B_ALIGN_HORIZONTAL_CENTER	= B_ALIGN_CENTER,

	B_ALIGN_HORIZONTAL_UNSET	= -1L,
	B_ALIGN_USE_FULL_WIDTH		= -2L
};

enum vertical_alignment {
	B_ALIGN_TOP					= 0x10L,
	B_ALIGN_MIDDLE				= 0x20,
	B_ALIGN_BOTTOM				= 0x30,

	B_ALIGN_VERTICAL_CENTER		= B_ALIGN_MIDDLE,

	B_ALIGN_VERTICAL_UNSET		= -1L,
	B_ALIGN_NO_VERTICAL			= B_ALIGN_VERTICAL_UNSET,
	B_ALIGN_USE_FULL_HEIGHT		= -2L
};


uint32			modifiers();


#endif // INTERFACE_DEFS_H
