/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef STROKE_PROPERTIES_H
#define STROKE_PROPERTIES_H

#include "Referenceable.h"
#include "SetProperty.h"

enum CapMode {
	ButtCap									= 0,
	SquareCap								= 1,
	RoundCap								= 2
};

enum JoinMode {
	MiterJoin								= 0,
	RoundJoin								= 1,
	BevelJoin								= 2,
};

class StrokeProperties : public Referenceable {
public:
	StrokeProperties()
		:
		fWidth(1.0f),
		fMiterLimit(4.0f),
		fSetProperties(0),
		fCapMode(ButtCap),
		fJoinMode(MiterJoin)
	{
	}

	StrokeProperties(float width)
		:
		fWidth(width),
		fMiterLimit(4.0f),
		fSetProperties(STROKE_WIDTH),
		fCapMode(ButtCap),
		fJoinMode(MiterJoin)
	{
	}

	StrokeProperties(float width, ::CapMode capMode)
		:
		fWidth(width),
		fMiterLimit(4.0f),
		fSetProperties(STROKE_WIDTH | STROKE_CAP_MODE),
		fCapMode(capMode),
		fJoinMode(MiterJoin)
	{
	}

	StrokeProperties(float width, ::JoinMode joinMode)
		:
		fWidth(width),
		fMiterLimit(4.0f),
		fSetProperties(STROKE_WIDTH | STROKE_JOIN_MODE),
		fCapMode(ButtCap),
		fJoinMode(joinMode)
	{
	}

	StrokeProperties(float width, ::CapMode capMode, ::JoinMode joinMode)
		:
		fWidth(width),
		fMiterLimit(4.0f),
		fSetProperties(STROKE_WIDTH | STROKE_CAP_MODE | STROKE_JOIN_MODE),
		fCapMode(capMode),
		fJoinMode(joinMode)
	{
	}

	StrokeProperties(float width, ::CapMode capMode, ::JoinMode joinMode,
			float miterLimit)
		:
		fWidth(width),
		fMiterLimit(miterLimit),
		fSetProperties(STROKE_WIDTH | STROKE_CAP_MODE | STROKE_JOIN_MODE
			| STROKE_MITER_LIMIT),
		fCapMode(capMode),
		fJoinMode(joinMode)
	{
	}

	StrokeProperties(const StrokeProperties& other)
	{
		*this = other;
	}

	bool operator==(const StrokeProperties& other) const
	{
		return fWidth == other.fWidth && fMiterLimit == other.fMiterLimit
			&& fSetProperties == other.fSetProperties
			&& fCapMode == other.fCapMode && fJoinMode == other.fJoinMode;
	}

	bool operator!=(const StrokeProperties& other) const
	{
		return !(*this == other);
	}

	inline float Width() const
	{
		return fWidth;
	}

	inline float MiterLimit() const
	{
		return fMiterLimit;
	}

	inline ::CapMode CapMode() const
	{
		return (::CapMode)fCapMode;
	}

	inline ::JoinMode JoinMode() const
	{
		return (::JoinMode)fJoinMode;
	}

	inline uint32 SetProperties() const
	{
		return fSetProperties;
	}

private:
	StrokeProperties& operator=(const StrokeProperties& other)
	{
		fWidth = other.fWidth;
		fMiterLimit = other.fMiterLimit;
		fSetProperties = other.fSetProperties;
		fCapMode = other.fCapMode;
		fJoinMode = other.fJoinMode;
		return *this;
	}

	float		fWidth;
	float		fMiterLimit;
	uint32		fSetProperties : 10;
	uint32		fCapMode : 2;
	uint32		fJoinMode : 2;
};

#endif // STROKE_PROPERTIES_H
