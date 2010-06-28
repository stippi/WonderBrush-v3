/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TRANSFORM_VIEW_STATE_H
#define TRANSFORM_VIEW_STATE_H

#include "ViewState.h"
#include "Transformable.h"


class TransformViewState : public ViewState {
public:
								TransformViewState(StateView* view);
	virtual						~TransformViewState();

			void				SetObjectToCanvasTransformation(
									const Transformable& transformation);
			void				SetAdditionalTransformation(
									const Transformable& transformation);

			void				TransformObjectToCanvas(BPoint* point) const;
			void				TransformCanvasToObject(BPoint* point) const;

			void				TransformObjectToView(BPoint* point,
									bool round) const;
			void				TransformViewToObject(BPoint* point) const;

			void				TransformObjectToCanvas(BRect* bounds) const;
			void				TransformObjectToView(BRect* bounds) const;

	virtual	float				ViewspaceRotation() const;

	inline	const Transformable& EffectiveTransformation() const
									{ return fEffectiveTransformation; }

private:
			void				_UpdateTransformation();

			Transformable		fObjectToCanvasTransformation;
			Transformable		fAdditionalTransformation;
			Transformable		fEffectiveTransformation;
};

#endif // TRANSFORM_VIEW_STATE_H
