#ifndef BACK_BUFFERED_STATE_VIEW_H
#define BACK_BUFFERED_STATE_VIEW_H

#include "StateView.h"

class BBitmap;

class BackBufferedStateView : public StateView {
 public:
								BackBufferedStateView(BRect frame,
									const char* name, uint32 resizingMode,
									uint32 flags);
#ifdef __HAIKU__
								BackBufferedStateView(const char* name,
									uint32 flags);
#endif

	virtual						~BackBufferedStateView();

	// StateView interface
	virtual	void				AttachedToWindow();
	virtual	void				FrameResized(float width, float height);
	virtual void				PlatformDraw(PlatformDrawContext& drawContext);

	// BackBufferedStateView
 protected:
	virtual	void				DrawBackgroundInto(BView* view,
									BRect updateRect);
	virtual	void				DrawInto(BView* view, BRect updateRect);
			void				SyncGraphicsState();
									// applies the current graphics state
									// onto the BView used to do the
									// offscreen drawing

			void				SetSyncToRetrace(bool sync);

 private:
			void				_AllocBackBitmap(float width,
												 float height);
			void				_FreeBackBitmap();


	BBitmap*					fOffscreenBitmap;
	BView*						fOffscreenView;
	bool						fSyncToRetrace;
};

#endif // BACK_BUFFERED_STATE_VIEW_H
