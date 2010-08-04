/*
 * Copyright 2009-2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef STROKE_PROPERTIES_H
#define STROKE_PROPERTIES_H

#include <agg_math_stroke.h>

#include "Referenceable.h"
#include "SetProperty.h"

enum CapMode {
	ButtCap									= agg::butt_cap,
	SquareCap								= agg::square_cap,
	RoundCap								= agg::round_cap
};

enum JoinMode {
	MiterJoin								= agg::miter_join,
	MiterJoinRevert							= agg::miter_join_revert,
	RoundJoin								= agg::round_join,
	BevelJoin								= agg::bevel_join,
	MiterJoinRound							= agg::miter_join_round
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

	StrokeProperties& operator=(const StrokeProperties& other)
	{
		fWidth = other.fWidth;
		fMiterLimit = other.fMiterLimit;
		fSetProperties = other.fSetProperties;
		fCapMode = other.fCapMode;
		fJoinMode = other.fJoinMode;
		return *this;
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

	inline size_t HashKey() const
	{
		// TODO: ...
		return 0;
	}

	template<typename Converter>
	inline void SetupAggConverter(Converter& converter) const
	{
		converter.width(fWidth);
		converter.line_cap(static_cast<agg::line_cap_e>(fCapMode));
		converter.line_join(static_cast<agg::line_join_e>(fJoinMode));
		converter.miter_limit(fMiterLimit);
	}

private:
	float		fWidth;
	float		fMiterLimit;
	uint32		fSetProperties : 10;
	uint32		fCapMode : 2;
	uint32		fJoinMode : 2;
};

#endif // STROKE_PROPERTIES_H
