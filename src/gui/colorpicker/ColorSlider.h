/*
 * Copyright 2001 Werner Freytag - please read to the LICENSE file
 *
 * Copyright 2002-2006, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 *
 */

#ifndef COLOR_SLIDER_H
#define COLOR_SLIDER_H

#include <Control.h>

#include "SelectedColorMode.h"

#define	MSG_COLOR_SLIDER	'ColS'

class BBitmap;

class ColorSlider : public BControl {
public:
								ColorSlider(SelectedColorMode mode,
									float value1, float value2,
									orientation dir = B_VERTICAL);
								ColorSlider(BPoint offsetPoint,
									SelectedColorMode mode,
									float value1, float value2,
									orientation dir = B_VERTICAL);
	virtual						~ColorSlider();

								// BControl
	virtual	BSize				MinSize();
	virtual	BSize				PreferredSize();
	virtual	BSize				MaxSize();

	virtual	void				AttachedToWindow();

	virtual	status_t			Invoke(BMessage* message = NULL);

	virtual	void				Draw(BRect updateRect);
	virtual	void				FrameResized(float width, float height);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 code,
										   const BMessage* dragMessage);

	virtual	void				SetValue(int32 value);

								// ColorSlider
			void				Update(int depth);

			bool				IsTracking() const
									{ return fMouseDown; }

			void				SetModeAndValues(SelectedColorMode mode,
									float value1, float value2);
			void				SetOtherValues(float value1, float value2);
			void				GetOtherValues(float* value1,
									float* value2) const;

			void				SetMarkerToColor( rgb_color color );

private:
			void				_Init(SelectedColorMode mode,
						 			float value1, float value2,
						 			orientation dir);

	static	int32				_UpdateThread(void* cookie);
	static	inline void			_DrawColorLineY(BView* view, float y,
									int r, int g, int b);
	static	inline void			_DrawColorLineX(BView* view, float x,
									int r, int g, int b);
			void				_TrackMouse(BPoint where);

private:
	SelectedColorMode			fMode;
	float						fFixedValue1;
	float						fFixedValue2;

	bool						fMouseDown;

	BBitmap*					fBgBitmap;
	BView*						fBgView;

	thread_id					fUpdateThread;
	port_id						fUpdatePort;

	orientation					fOrientation;
};

#endif // COLOR_SLIDER_H
