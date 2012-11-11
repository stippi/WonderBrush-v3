/*
 * Copyright 2001 Werner Freytag - please read to the LICENSE file
 *
 * Copyright 2002-2006, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 *
 */

#ifndef COLOR_FIELD_H
#define COLOR_FIELD_H

#include <Control.h>

#include "PlatformViewMixin.h"
#include "SelectedColorMode.h"


enum {
	MSG_COLOR_FIELD		= 'ColF',
};

class BBitmap;

class ColorField : public PlatformViewMixin<BControl> {
public:
								ColorField(BPoint offset_point,
									SelectedColorMode mode, float fixedValue,
									orientation orient = B_VERTICAL,
									border_style border = B_FANCY_BORDER);

								ColorField(SelectedColorMode mode,
									float fixedValue,
									orientation orient = B_VERTICAL,
									border_style border = B_FANCY_BORDER);

	virtual						~ColorField();

	// BControl interface
	virtual	BSize				MinSize();
	virtual	BSize				PreferredSize();
	virtual	BSize				MaxSize();

	virtual	status_t			Invoke(BMessage* message = NULL);

	virtual	void				AttachedToWindow();
	virtual	void				PlatformDraw(PlatformDrawContext& drawContext);
	virtual	void				FrameResized(float width, float height);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 code,
									const BMessage* dragMessage);

	// ColorField
			void				SetModeAndValue(SelectedColorMode mode,
									float fixedValue);
			void				SetFixedValue(float fixedValue);
			float				FixedValue() const
									{ return fFixedValue; }

			void				SetMarkerToColor(rgb_color color);
			void				PositionMarkerAt(BPoint where);

			float				Width() const;
			float				Height() const;
			bool				IsTracking() const
									{ return fMouseDown; }

private:
			class PlatformDelegate;

private:
			void				_Init(SelectedColorMode mode,
									float fixedValue, orientation orient,
									border_style border);

			void				_AllocBitmap(int32 width, int32 height);
			void				_Update();
			BRect				_BitmapRect() const;
			void				_FillBitmap(BBitmap* bitmap,
									SelectedColorMode mode,
									float fixedValue, orientation orient) const;

private:
	PlatformDelegate*			fPlatformDelegate;

	SelectedColorMode			fMode;
	float						fFixedValue;
	orientation					fOrientation;
	border_style				fBorderStyle;

	BPoint						fMarkerPosition;
	rgb_color					fMarkerColor;
	bool						fMouseDown;

	BBitmap*					fBitmap;
	bool						fBitmapDirty;
};

#endif // COLOR_FIELD_H
