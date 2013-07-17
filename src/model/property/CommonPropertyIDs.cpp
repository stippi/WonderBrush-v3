/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "CommonPropertyIDs.h"

#include <stdio.h>


// name_for_id
const char*
name_for_id(int32 id)
{
	const char* name = NULL;
	switch (id) {
		case PROPERTY_NAME:
			name = "Name";
			break;

		case PROPERTY_OPACITY:
			name = "Opacity";
			break;
		case PROPERTY_BLENDING_MODE:
			name = "Mode";
			break;
		case PROPERTY_COLOR:
			name = "Color";
			break;

		case PROPERTY_WIDTH:
			name = "Width";
			break;
		case PROPERTY_HEIGHT:
			name = "Height";
			break;

		case PROPERTY_CAP_MODE:
			name = "Caps";
			break;
		case PROPERTY_JOIN_MODE:
			name = "Joins";
			break;
		case PROPERTY_MITER_LIMIT:
			name = "Miter Limit";
			break;
		case PROPERTY_STROKE_SHORTEN:
			name = "Shorten";
			break;
		case PROPERTY_STROKE_POSITION:
			name = "Position";
			break;

		case PROPERTY_CLOSED:
			name = "Closed";
			break;
		case PROPERTY_PATH:
			name = "Path";
			break;

		case PROPERTY_HINTING:
			name = "Rounding";
			break;
		case PROPERTY_MIN_VISIBILITY_SCALE:
			name = "Min LOD";
			break;
		case PROPERTY_MAX_VISIBILITY_SCALE:
			name = "Max LOD";
			break;

		case PROPERTY_TRANSLATION_X:
			name = "Translation X";
			break;
		case PROPERTY_TRANSLATION_Y:
			name = "Translation Y";
			break;
		case PROPERTY_ROTATION:
			name = "Rotation";
			break;
		case PROPERTY_SCALE_X:
			name = "Scale X";
			break;
		case PROPERTY_SCALE_Y:
			name = "Scale Y";
			break;

		case PROPERTY_OFFSET_X:
			name = "Offset X";
			break;
		case PROPERTY_OFFSET_Y:
			name = "Offset Y";
			break;

		case PROPERTY_DETECT_ORIENTATION:
			name = "Detect Orient.";
			break;

		case PROPERTY_FILTER_RADIUS:
			name = "Filter Radius";
			break;
		case PROPERTY_HSV_HUE:
			name = "Hue";
			break;
		case PROPERTY_SATURATION:
			name = "Saturation";
			break;
		case PROPERTY_HSV_VALUE:
			name = "Value";
			break;

		case PROPERTY_GROUP_STROKE_PAINT:
			name = "Stroke";
			break;
		case PROPERTY_GROUP_FILL_PAINT:
			name = "Fill";
			break;

		case PROPERTY_FILL_PAINT_TYPE:
			name = "Type";
			break;
		case PROPERTY_FILL_PAINT_COLOR:
			name = "Color";
			break;
		case PROPERTY_FILL_PAINT_GRADIENT:
			name = "Gradient";
			break;
		case PROPERTY_FILL_PAINT_PATTERN:
			name = "Pattern";
			break;

		case PROPERTY_STROKE_PAINT_TYPE:
			name = "Type";
			break;
		case PROPERTY_STROKE_PAINT_COLOR:
			name = "Color";
			break;
		case PROPERTY_STROKE_PAINT_GRADIENT:
			name = "Gradient";
			break;
		case PROPERTY_STROKE_PAINT_PATTERN:
			name = "Pattern";
			break;

		default:
			name = "<unkown property>";
			break;
	}
	return name;
}

