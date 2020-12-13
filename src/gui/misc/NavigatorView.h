/*
 * Copyright 2010-2020 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */
#ifndef NAVIGATOR_VIEW_H
#define NAVIGATOR_VIEW_H

#include <Locker.h>
#include <String.h>

#include "PlatformViewMixin.h"


#define NAVIGATOR_VIEW_USE_BEAUTIFUL_DOWN_SCALING 0


class Document;
class RenderManager;


class NavigatorView : public PlatformViewMixin<BView> {
public:
								NavigatorView(Document* document,
									RenderManager* manager);
	virtual						~NavigatorView();

	// BView
	virtual	void				MessageReceived(BMessage* message);

	virtual	void				Pulse();
	virtual	void				AttachedToWindow();
	virtual	void				FrameResized(float width, float height);
	virtual	void				PlatformDraw(PlatformDrawContext& drawContext);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* dragMessage);

	virtual	BSize				MinSize();
	virtual	BSize				MaxSize();
	virtual	BSize				PreferredSize();
	virtual	bool				HasHeightForWidth();
	virtual	void				GetHeightForWidth(float width, float* min,
									float* max, float* preferred);

private:
			class PlatformDelegate;

private:
			BRect				_ImageBounds() const;
			BRect				_VisibleRect() const;
			BRect				_VisibleRect(const BRect& imageBounds) const;
			
			void				_AllocateBitmap(BRect bounds);
			void				_RescaleBitmap(const BBitmap* source,
									const BBitmap* dest, BRect area);

	static	int32				_RescaleThreadEntry(void* cookie);
			int32				_RescaleThread();

private:
			Document*			fDocument;
			RenderManager*		fRenderManager;

			BBitmap*			fScaledBitmap;
			BRect				fBitmapBounds;
			BRect				fDirtyDisplayArea;

			BLocker				fRescaleLock;
			thread_id			fRescaleThread;

			PlatformDelegate*	fPlatformDelegate;

			BPoint				fDragStart;
			BRect				fDragStartVisibleRect;
			bool				fDragging;

			enum {
				DRAG_VISIBLE_RECT	= 0,
				SET_VISIBLE_RECT,
				IGNORE_CLICK
			};
			uint32				fDragMode;
			void				_SetDragMode(uint32 mode);
			void				_MoveVisibleRect(BPoint offset);
};

#endif // NAVIGATOR_VIEW_H
