/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef SET_PROPERTY_H
#define SET_PROPERTY_H

enum {
	STROKE_PAINT								= 1 << 0,
	STROKE_WIDTH								= 1 << 1,
	STROKE_JOIN_MODE							= 1 << 2,
	STROKE_CAP_MODE								= 1 << 3,
	STROKE_MITER_LIMIT							= 1 << 4,

	FILL_PAINT									= 1 << 10,
	FILL_MODE									= 1 << 11,
};

#endif // SET_PROPERTY_H
