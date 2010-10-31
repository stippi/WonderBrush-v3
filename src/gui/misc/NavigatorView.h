/*
 * Copyright 2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */
#ifndef NAVIGATOR_VIEW_H
#define NAVIGATOR_VIEW_H

#include <Locker.h>
#include <String.h>
#include <View.h>

class Document;
class RenderManager;

class NavigatorView : public BView {
public:
								NavigatorView(Document* document,
									RenderManager* manager);
	virtual						~NavigatorView();

	// BView
	virtual	void				MessageReceived(BMessage* message);

	virtual	void				Pulse();
	virtual	void				AttachedToWindow();
	virtual	void				FrameResized(float width, float height);
	virtual	void				Draw(BRect updateRect);

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
			BRect				_IconBounds() const;
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
};

#endif // NAVIGATOR_VIEW_H
