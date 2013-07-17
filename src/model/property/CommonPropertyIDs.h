/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#ifndef COMMON_PROPERTY_IDS_H
#define COMMON_PROPERTY_IDS_H

#include <SupportDefs.h>

enum {
	PROPERTY_NAME						= 'name',

	PROPERTY_OPACITY					= 'alpa',
	PROPERTY_BLENDING_MODE				= 'blnd',
	PROPERTY_COLOR						= 'colr',

	PROPERTY_WIDTH						= 'wdth',
	PROPERTY_HEIGHT						= 'hght',

	PROPERTY_CAP_MODE					= 'cpmd',
	PROPERTY_JOIN_MODE					= 'jnmd',
	PROPERTY_MITER_LIMIT				= 'mtlm',
	PROPERTY_STROKE_SHORTEN				= 'srtn',
	PROPERTY_STROKE_POSITION			= 'stps',

	PROPERTY_CLOSED						= 'clsd',
	PROPERTY_PATH						= 'path',

	PROPERTY_HINTING					= 'hntg',
	PROPERTY_MIN_VISIBILITY_SCALE		= 'mnld',
	PROPERTY_MAX_VISIBILITY_SCALE		= 'mxld',

	PROPERTY_TRANSLATION_X				= 'trnx',
	PROPERTY_TRANSLATION_Y				= 'trny',
	PROPERTY_ROTATION					= 'rotn',
	PROPERTY_SCALE_X					= 'sclx',
	PROPERTY_SCALE_Y					= 'scly',

	PROPERTY_OFFSET_X					= 'ofsx',
	PROPERTY_OFFSET_Y					= 'ofsy',

	PROPERTY_DETECT_ORIENTATION			= 'ador',

	PROPERTY_FILTER_RADIUS				= 'flrd',

	PROPERTY_HSV_HUE					= 'hue ',
	PROPERTY_SATURATION					= 'strn',
	PROPERTY_HSV_VALUE					= 'vlue',

	PROPERTY_GROUP_STROKE_PAINT			= 'strk',
	PROPERTY_GROUP_FILL_PAINT			= 'fill',

	PROPERTY_STROKE_PAINT_TYPE			= 'ptst',
	PROPERTY_STROKE_PAINT_COLOR			= 'ptsc',
	PROPERTY_STROKE_PAINT_GRADIENT		= 'ptsg',
	PROPERTY_STROKE_PAINT_PATTERN		= 'ptsp',

	PROPERTY_FILL_PAINT_TYPE			= 'ptft',
	PROPERTY_FILL_PAINT_COLOR			= 'ptfc',
	PROPERTY_FILL_PAINT_GRADIENT		= 'ptfg',
	PROPERTY_FILL_PAINT_PATTERN			= 'ptfp',
};


const char*		name_for_id(int32 id);

#endif // COMMON_PROPERTY_IDS_H


