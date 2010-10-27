/*
 * Copyright 2001-2009, Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2001-2009, Ingo Weinhold <ingo_weinhold@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

/** Simple extension to the Scrollable class to simplify the creation
    of derived view classes. */

#ifndef SCROLLABLE_VIEW_H
#define SCROLLABLE_VIEW_H

#include "Scrollable.h"

class ScrollableView : public Scrollable {
 public:
								ScrollableView();
	virtual						~ScrollableView();

protected:
	virtual	void				ScrollOffsetChanged(BPoint oldOffset,
													BPoint newOffset);
};


#endif	// SCROLLABLE_VIEW_H
