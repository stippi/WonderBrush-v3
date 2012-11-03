/*
 * Copyright 2009-2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef STROKE_PROPERTIES_H
#define STROKE_PROPERTIES_H

#include <agg_math_stroke.h>

#include "BaseObject.h"
#include "SetProperty.h"

enum CapMode {
	ButtCap						= agg::butt_cap,
	SquareCap					= agg::square_cap,
	RoundCap					= agg::round_cap
};

enum JoinMode {
	MiterJoin					= agg::miter_join,
	MiterJoinRevert				= agg::miter_join_revert,
	RoundJoin					= agg::round_join,
	BevelJoin					= agg::bevel_join,
	MiterJoinRound				= agg::miter_join_round
};

enum StrokePosition {
	CenterStroke				= 0,
	OutsideStroke				= 1,
	InsideStroke				= 2,
};

class StrokeProperties : public BaseObject {
public:
								StrokeProperties();
								StrokeProperties(float width);
								StrokeProperties(float width,
									::CapMode capMode);
								StrokeProperties(float width,
									::JoinMode joinMode);
								StrokeProperties(float width,
									::CapMode capMode, ::JoinMode joinMode);
								StrokeProperties(float width,
									::CapMode capMode, ::JoinMode joinMode,
									float miterLimit);
								StrokeProperties(
									const StrokeProperties& other);

								~StrokeProperties();

	// BaseObject interface
	virtual	status_t			Unarchive(const BMessage* archive);
	virtual	status_t			Archive(BMessage* into,
									bool deep = true) const;
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);
	virtual	const char*			DefaultName() const;

	// StrokeProperties
			StrokeProperties&	operator=(const StrokeProperties& other);

			bool				operator==(
									const StrokeProperties& other) const;

			bool				operator!=(
									const StrokeProperties& other) const;

			void				SetWidth(float width);
	inline	float				Width() const
									{ return fWidth; }

			void				SetMiterLimit(float miterLimit);
	inline	float				MiterLimit() const
									{ return fMiterLimit; }

			void				SetCapMode(::CapMode capMode);
	inline	::CapMode			CapMode() const
									{ return (::CapMode)fCapMode; }

			void				SetJoinMode(::JoinMode joinMode);
	inline	::JoinMode			JoinMode() const
									{ return (::JoinMode)fJoinMode; }

			void				SetStrokePosition(::StrokePosition position);
	inline	::StrokePosition	StrokePosition() const
									{ return
										(::StrokePosition)fStrokePosition; }

	inline	uint32				SetProperties() const
									{ return fSetProperties; }

			size_t				HashKey() const;

			template<typename Converter>
			void				SetupAggConverter(Converter& converter) const;

private:
			float				fWidth;
			float				fMiterLimit;
			uint32				fSetProperties : 10;
			uint32				fCapMode : 2;
			uint32				fJoinMode : 3;
			uint32				fStrokePosition : 2;
};

// SetupAggConverter
template<typename Converter>
void
StrokeProperties::SetupAggConverter(Converter& converter) const
{
	converter.width(fWidth);
	converter.line_cap(static_cast<agg::line_cap_e>(fCapMode));
	converter.line_join(static_cast<agg::line_join_e>(fJoinMode));
	converter.miter_limit(fMiterLimit);
}

#endif // STROKE_PROPERTIES_H
