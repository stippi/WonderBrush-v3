/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef LAYOUT_STATE_H
#define LAYOUT_STATE_H

#include "Style.h"
#include "Transformable.h"

// Represents a graphical state at a certain point during the layout
// of objects.

class LayoutState {
public:
								LayoutState();
								LayoutState(LayoutState* previous);
								~LayoutState();

			LayoutState&		operator=(const LayoutState& other);

			LayoutState*		Previous;

			Transformable		Matrix;

			void				SetFillPaint(Paint* paint);
			void				SetStrokePaint(Paint* paint);

	inline	const Paint*		FillPaint() const
									{ return fFillPaint; }
	inline	const Paint*		StrokePaint() const
									{ return fStrokePaint; }

private:
			void				_SetPaint(Paint*& member, Paint* paint);

			Paint*				fFillPaint;
			Paint*				fStrokePaint;
};

#endif // LAYOUT_STATE_H
