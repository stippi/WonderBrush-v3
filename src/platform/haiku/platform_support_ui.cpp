#include "support_ui.h"

#include <View.h>

#include "PlatformViewMixin.h"


// stroke_frame
void
stroke_frame(PlatformDrawContext& drawContext, BRect r, rgb_color left,
	rgb_color top, rgb_color right, rgb_color bottom)
{
	BView* v = drawContext.View();

	if (v && r.IsValid()) {
		v->BeginLineArray(4);
			v->AddLine(BPoint(r.left, r.bottom),
					   BPoint(r.left, r.top), left);
			v->AddLine(BPoint(r.left + 1.0, r.top),
					   BPoint(r.right, r.top), top);
			v->AddLine(BPoint(r.right, r.top + 1.0),
					   BPoint(r.right, r.bottom), right);
			v->AddLine(BPoint(r.right - 1.0, r.bottom),
					   BPoint(r.left + 1.0, r.bottom), bottom);
		v->EndLineArray();
	}
}
