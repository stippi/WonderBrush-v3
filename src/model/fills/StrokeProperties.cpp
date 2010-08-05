/*
 * Copyright 2009-2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "StrokeProperties.h"

// constructor
StrokeProperties::StrokeProperties()
	: fWidth(1.0f)
	, fMiterLimit(4.0f)
	, fSetProperties(0)
	, fCapMode(ButtCap)
	, fJoinMode(MiterJoin)
{
}

// constructor
StrokeProperties::StrokeProperties(float width)
	: fWidth(width)
	, fMiterLimit(4.0f)
	, fSetProperties(STROKE_WIDTH)
	, fCapMode(ButtCap)
	, fJoinMode(MiterJoin)
{
}

// constructor
StrokeProperties::StrokeProperties(float width, ::CapMode capMode)
	: fWidth(width)
	, fMiterLimit(4.0f)
	, fSetProperties(STROKE_WIDTH | STROKE_CAP_MODE)
	, fCapMode(capMode)
	, fJoinMode(MiterJoin)
{
}

// constructor
StrokeProperties::StrokeProperties(float width, ::JoinMode joinMode)
	: fWidth(width)
	, fMiterLimit(4.0f)
	, fSetProperties(STROKE_WIDTH | STROKE_JOIN_MODE)
	, fCapMode(ButtCap)
	, fJoinMode(joinMode)
{
}

// constructor
StrokeProperties::StrokeProperties(float width, ::CapMode capMode,
		::JoinMode joinMode)
	: fWidth(width)
	, fMiterLimit(4.0f)
	, fSetProperties(STROKE_WIDTH | STROKE_CAP_MODE | STROKE_JOIN_MODE)
	, fCapMode(capMode)
	, fJoinMode(joinMode)
{
}

// constructor
StrokeProperties::StrokeProperties(float width, ::CapMode capMode,
		::JoinMode joinMode, float miterLimit)
	: fWidth(width)
	, fMiterLimit(miterLimit)
	, fSetProperties(STROKE_WIDTH | STROKE_CAP_MODE | STROKE_JOIN_MODE
		| STROKE_MITER_LIMIT)
	, fCapMode(capMode)
	, fJoinMode(joinMode)
{
}

// constructor
StrokeProperties::StrokeProperties(const StrokeProperties& other)
{
	*this = other;
}

// operator=
StrokeProperties&
StrokeProperties::operator=(const StrokeProperties& other)
{
	fWidth = other.fWidth;
	fMiterLimit = other.fMiterLimit;
	fSetProperties = other.fSetProperties;
	fCapMode = other.fCapMode;
	fJoinMode = other.fJoinMode;
	return *this;
}

// operator==
bool
StrokeProperties::operator==(const StrokeProperties& other) const
{
	return fWidth == other.fWidth && fMiterLimit == other.fMiterLimit
		&& fSetProperties == other.fSetProperties
		&& fCapMode == other.fCapMode && fJoinMode == other.fJoinMode;
}

// operator!=
bool
StrokeProperties::operator!=(const StrokeProperties& other) const
{
	return !(*this == other);
}

// HashKey
size_t
StrokeProperties::HashKey() const
{
	// TODO: ...
	return 0;
}

