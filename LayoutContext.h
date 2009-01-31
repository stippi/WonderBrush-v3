/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef LAYOUT_CONTEXT_H
#define LAYOUT_CONTEXT_H

// This class should become a graphics state stack, usable by objects
// to layout themselves based on inherited propertis and obtain absolute
// values for these properties that they can cache for out-of-order
// rendering.

class LayoutContext {
public:
								LayoutContext();
	virtual						~LayoutContext();
};

#endif // LAYOUT_CONTEXT_H
