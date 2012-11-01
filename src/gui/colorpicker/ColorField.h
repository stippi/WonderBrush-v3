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

#include "SelectedColorMode.h"

enum {
	MSG_COLOR_FIELD		= 'ColF',
};

class BBitmap;

class ColorField : public BControl {
public:
								ColorField(BPoint offset_point,
									SelectedColorMode mode, float fixedValue,
									orientation orient = B_VERTICAL);

								ColorField(SelectedColorMode mode,
									float fixedValue,
									orientation orient = B_VERTICAL);

	virtual						~ColorField();

	// BControl interface
	virtual	BSize				MinSize();
	virtual	BSize				PreferredSize();
	virtual	BSize				MaxSize();

	virtual	status_t			Invoke(BMessage* message = NULL);

	virtual	void				AttachedToWindow();
	virtual	void				Draw(BRect updateRect);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 code,
									const BMessage* dragMessage);

	// ColorField
			void				Update(int depth);

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
			void				_Init(SelectedColorMode mode,
									float fixedValue, orientation orient);

	static	status_t			_UpdateThread(void* data);
			void				_DrawBorder();

private:
	SelectedColorMode			fMode;
	float						fFixedValue;
	orientation					fOrientation;

	BPoint						fMarkerPosition;
	BPoint						fLastMarkerPosition;

	bool						fMouseDown;

	BBitmap*					fBgBitmap[2];
	BView*						fBgView[2];

	thread_id					fUpdateThread;
	port_id						fUpdatePort;
};

#endif // COLOR_FIELD_H
