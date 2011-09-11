/*
 * Copyright 2006-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef CHANNEL_TRANSFORM_H
#define CHANNEL_TRANSFORM_H

#include "Transformable.h"

class ChannelTransform : public Transformable {
public:
								ChannelTransform();
								ChannelTransform(const ChannelTransform& other);
	virtual						~ChannelTransform();

	// ChannelTransform
	virtual	void				Update(bool deep = true) {}

			void				SetTransformation(const Transformable& other);

			void				SetTransformation(BPoint pivot,
									BPoint translation, double rotation,
									double xScale, double yScale);

			void				SetPivot(const BPoint& pivot);

	virtual	void				TranslateBy(BPoint offset);
	virtual	void				RotateBy(BPoint origin, double degrees);
	virtual	void				ScaleBy(BPoint origin, double scaleX,
									double scaleY);

			void				ScaleBy(double xScale, double yScale);
			void				RotateBy(double degrees);

			void				SetTranslationAndScale(BPoint offset,
									double xScale, double yScale);

	virtual	void				Reset();

	inline	const BPoint&		Pivot() const
									{ return fPivot; }
	inline	const BPoint&		Translation() const
									{ return fTranslation; }
	inline	double				LocalRotation() const
									{ return fRotation; }
	inline	double				LocalXScale() const
									{ return fXScale; }
	inline	double				LocalYScale() const
									{ return fYScale; }

			bool				operator==(
									const ChannelTransform& other) const;
			ChannelTransform&	operator=(const ChannelTransform& other);

private:
			void				_UpdateMatrix(bool deep = true);

			BPoint				fPivot;
			BPoint				fTranslation;
			double				fRotation;
			double				fXScale;
			double				fYScale;
};

#endif // CHANNEL_TRANSFORM_H

