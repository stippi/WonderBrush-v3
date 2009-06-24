// ScrollableView.h
//
// Simple extension to the Scrollable class to simplify the creation
// of derived view classes.

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
