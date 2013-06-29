/*
 * Copyright 2006-2012, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */

#include "SwatchView.h"
#include "SwatchViewPlatformDelegate.h"

#include <stdio.h>

#include <Bitmap.h>
#include <Cursor.h>
#include <Looper.h>
#include <Message.h>
#include <TypeConstants.h>
#include <Window.h>

#include "cursors.h"
#include "ui_defines.h"
#include "support.h"
#include "support_ui.h"

#define DRAG_INIT_DIST 10.0

// constructor
SwatchView::SwatchView(const char* name, BMessage* message,
	BHandler* target, rgb_color color, float width, float height,
	border_style border)
	: PlatformViewMixin<BView>(BRect(0.0, 0.0, width, height), name,
			B_FOLLOW_NONE, B_WILL_DRAW),
	  fPlatformDelegate(new PlatformDelegate(this)),
	  fColor(color),
	  fTrackingStart(-1.0, -1.0),
	  fActive(false),
	  fDropInvokes(false),
	  fClickMessage(message),
	  fDroppedMessage(NULL),
	  fTarget(target),
	  fWidth(width),
	  fHeight(height),
	  fBorderStyle(border)
{
}

// destructor
SwatchView::~SwatchView()
{
	delete fClickMessage;
	delete fDroppedMessage;
	delete fPlatformDelegate;
}

inline void
blend_color(rgb_color& a, const rgb_color& b, float alpha)
{
	float alphaInv = 1.0 - alpha;
	a.red = (uint8)(b.red * alphaInv + a.red * alpha);
	a.green = (uint8)(b.green * alphaInv + a.green * alpha);
	a.blue = (uint8)(b.blue * alphaInv + a.blue * alpha);
}

// PlatformDraw
void
SwatchView::PlatformDraw(PlatformDrawContext& drawContext)
{
	DrawSwatch(Bounds(), drawContext);
}

// MessageReceived
void
SwatchView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_PASTE: {
			rgb_color color;
			if (restore_color_from_message(message,
										   color) >= B_OK) {
				SetColor(color);
				_Invoke(fDroppedMessage);
			}
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}

// MouseDown
void
SwatchView::MouseDown(BPoint where)
{
	if (Bounds().Contains(where))
		fTrackingStart = where;
}

// MouseUp
void
SwatchView::MouseUp(BPoint where)
{
	if (Bounds().Contains(where)
		&& Bounds().Contains(fTrackingStart))
		_Invoke(fClickMessage);
	fTrackingStart.x = -1.0;
	fTrackingStart.y = -1.0;
}

// MouseMoved
void
SwatchView::MouseMoved(BPoint where, uint32 transit,
					   const BMessage* dragMessage)
{
	if (transit == B_ENTERED_VIEW) {
		BCursor cursor(kDropperCursor);
		SetViewCursor(&cursor, true);
	}
	if (Bounds().Contains(fTrackingStart)) {
		if (point_point_distance(where, fTrackingStart)
			> DRAG_INIT_DIST || transit == B_EXITED_VIEW) {
			_DragColor();
			fTrackingStart.x = -1.0;
			fTrackingStart.y = -1.0;
		}
	}
}

// SetColor
void
SwatchView::SetColor(rgb_color color)
{
	fColor = color;
	fPlatformDelegate->ColorChanged(fColor);
	Invalidate();
}

// SetClickedMessage
void
SwatchView::SetClickedMessage(BMessage* message)
{
	delete fClickMessage;
	fClickMessage = message;
}

// SetDroppedMessage
void
SwatchView::SetDroppedMessage(BMessage* message)
{
	delete fDroppedMessage;
	fDroppedMessage = message;
}

// DrawSwatch
void
SwatchView::DrawSwatch(BRect r, PlatformDrawContext& drawContext)
{
	rgb_color colorLight = tint_color(fColor, B_LIGHTEN_1_TINT);
	rgb_color colorShadow = tint_color(fColor, B_DARKEN_1_TINT);

	if (fColor.alpha < 255) {
		// left/top
		float alpha = fColor.alpha / 255.0;

		rgb_color h = colorLight;
		blend_color(h, kAlphaHigh, alpha);
		rgb_color l = colorLight;
		blend_color(l, kAlphaLow, alpha);

		if (fBorderStyle == B_PLAIN_BORDER) {
			rgb_color leftTopHighColor = h;
			rgb_color leftTopLowColor = l;

			h = colorShadow;
			blend_color(h, kAlphaHigh, alpha);
			l = colorShadow;
			blend_color(l, kAlphaLow, alpha);

			fPlatformDelegate->DrawDottedBorder(drawContext, r,
				leftTopHighColor, leftTopLowColor, h, l);

			r.InsetBy(1.0, 1.0);
		}

		// fill
		h = fColor;
		blend_color(h, kAlphaHigh, alpha);
		l = fColor;
		blend_color(l, kAlphaLow, alpha);

		fPlatformDelegate->FillDottedRect(drawContext, r, h, l);
	} else {
		if (fBorderStyle == B_PLAIN_BORDER) {
			fPlatformDelegate->DrawPlainBorder(drawContext, r, colorLight,
				colorShadow);
			r.InsetBy(1.0, 1.0);
		}
		fPlatformDelegate->FillPlainRect(drawContext, r, fColor);
	}
}

// _Invoke
void
SwatchView::_Invoke(const BMessage* _message)
{
	if (_message) {
		BHandler* target = fTarget ? fTarget
							: dynamic_cast<BHandler*>(Window());
		BLooper* looper;
		if (target && (looper = target->Looper())) {
			BMessage message(*_message);
			message.AddPointer("be:source", (void*)this);
			message.AddInt64("be:when", system_time());
			message.AddBool("begin", true);
			store_color_in_message(&message, fColor);
			looper->PostMessage(&message, target);
		}
	}
}

// _DragColor
void
SwatchView::_DragColor()
{
	BBitmap *bitmap = new BBitmap(BRect(0.0, 0.0, 15.0, 15.0), B_RGBA32);
	BMessage message = make_color_drop_message(fColor, bitmap);

	DragMessage(&message, bitmap, B_OP_ALPHA, BPoint(9.0, 9.0));
}

