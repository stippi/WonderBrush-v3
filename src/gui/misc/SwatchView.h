/*
 * Copyright 2006-2012, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */

#ifndef SWATCH_VIEW_H
#define SWATCH_VIEW_H


#include <View.h>

#include "PlatformViewMixin.h"


class SwatchView : public PlatformViewMixin<BView> {
public:
								SwatchView(const char* name, BMessage* message,
									BHandler* target, rgb_color color,
									float width = 24.0, float height = 24.0,
									border_style border = B_PLAIN_BORDER);
	virtual						~SwatchView();

								// BView
	virtual	void				PlatformDraw(PlatformDrawContext& drawContext);
	virtual	void				MessageReceived(BMessage* message);

	virtual	BSize				MinSize();
	virtual	BSize				PreferredSize();
	virtual	BSize				MaxSize();

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* dragMessage);

								// SwatchView
			void				SetColor(rgb_color color);
			rgb_color			Color() const
									{ return fColor; }

			void				SetClickedMessage(BMessage* message);
			void				SetDroppedMessage(BMessage* message);

private:
			class PlatformDelegate;

private:
			void				_Invoke(const BMessage* message);
			void				_DragColor();

protected:
			void				DrawSwatch(BRect area,
									PlatformDrawContext& drawContext);

private:
			PlatformDelegate*	fPlatformDelegate;

			rgb_color			fColor;
			BPoint				fTrackingStart;
			bool				fActive;
			bool				fDropInvokes;

			BMessage*			fClickMessage;
			BMessage*			fDroppedMessage;
			BHandler*			fTarget;

			float				fWidth;
			float				fHeight;
			border_style		fBorderStyle;
};

#endif // SWATCH_VIEW_H
