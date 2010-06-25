/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef LAYOUT_CONTEXT_H
#define LAYOUT_CONTEXT_H

#include "LayoutState.h"

// This class should become a graphics state stack, usable by objects
// to layout themselves based on inherited propertis and obtain absolute
// values for these properties that they can cache for out-of-order
// rendering.

class LayoutContext {
public:
								LayoutContext(LayoutState* initialState);
	virtual						~LayoutContext();

			void				Init(double zoomLevel);

			void				PushState(LayoutState* state);
			void				PopState();

	inline	LayoutState*		State() const
									{ return fCurrentState; }

			void				SetTransformation(const Transformable& matrix);

	inline	double				ZoomLevel() const
									{ return fZoomLevel; }

private:
			LayoutState*		fCurrentState;
			double				fZoomLevel;
};

#endif // LAYOUT_CONTEXT_H
