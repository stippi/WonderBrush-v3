#ifndef BACK_BUFFERED_STATE_VIEW_H
#define BACK_BUFFERED_STATE_VIEW_H

#include "StateView.h"

class BBitmap;

class BackBufferedStateView : public StateView {
 public:
								BackBufferedStateView(BRect frame,
									const char* name, uint32 resizingMode,
									uint32 flags);
	virtual						~BackBufferedStateView();

	// StateView interface
	virtual	void				FrameResized(float width, float height);
	virtual	void				Draw(BRect updateRect);

	// BackBufferedStateView
 protected:
	virtual	void				DrawBackgroundInto(BView* view,
									BRect updateRect);
	virtual	void				DrawInto(BView* view, BRect updateRect);
			void				SyncGraphicsState();
									// applies the current graphics state
									// onto the BView used to do the
									// offscreen drawing

 private:
			void				_AllocBackBitmap(float width,
												 float height);
			void				_FreeBackBitmap();


	BBitmap*					fOffscreenBitmap;
	BView*						fOffscreenView;
};

#endif // BACK_BUFFERED_STATE_VIEW_H
