// ScrollableView.cpp

#include <View.h>

#include "ScrollableView.h"

// constructor
ScrollableView::ScrollableView()
	: Scrollable()
{
}

// destructor
ScrollableView::~ScrollableView()
{
}

// ScrollOffsetChanged
void
ScrollableView::ScrollOffsetChanged(BPoint oldOffset, BPoint newOffset)
{
	if (BView* view = dynamic_cast<BView*>(this)) {
		// We keep it simple: The part of the data rect we shall show now
		// has existed before as well (even if partially or completely
		// obscured), so we let CopyBits() do the messy details.
		BRect bounds(view->Bounds());
		view->CopyBits(bounds.OffsetByCopy(newOffset - oldOffset), bounds);
		// move our children
		for (int32 i = 0; BView* child = view->ChildAt(i); i++)
			child->MoveTo(child->Frame().LeftTop() + oldOffset - newOffset);
	}
}

