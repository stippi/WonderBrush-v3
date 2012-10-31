#ifndef NAVIGATOR_VIEW_PLATFORM_DELEGATE_H
#define NAVIGATOR_VIEW_PLATFORM_DELEGATE_H


#include "NavigatorView.h"

#include <QBrush>


class NavigatorView::PlatformDelegate {
public:
								PlatformDelegate(NavigatorView* view);

			void				DrawBitmap(PlatformDrawContext& drawContext,
									const BBitmap* bitmap,
									const BRect& iconBounds);
			void				DrawBackground(PlatformDrawContext& drawContext,
									const BRegion& region);
			void				DrawRect(PlatformDrawContext& drawContext,
									const BRect& visibleRect,
									const BRect& iconBounds);

private:
			NavigatorView*		fView;
};


#endif // NAVIGATOR_VIEW_PLATFORM_DELEGATE_H
