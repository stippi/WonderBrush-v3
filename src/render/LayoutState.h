/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef LAYOUT_STATE_H
#define LAYOUT_STATE_H

#include "Transformable.h"

// Represents a graphical state at a certain point during the layout
// of objects.

class LayoutState {
public:
								LayoutState();
								LayoutState(LayoutState* previous);
								~LayoutState();

	LayoutState*				Previous;

	Transformable				Matrix;
};

#endif // LAYOUT_STATE_H
