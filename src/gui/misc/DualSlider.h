/*
 * Copyright 2002-2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef DUAL_SLIDER_H
#define DUAL_SLIDER_H

#include <View.h>
#include <String.h>

class DualSlider : public BView {
public:
								DualSlider(const char* name,
									const char* label = NULL,
									BMessage* valueMessage = NULL,
									BMessage* controlMessage = NULL,
									BHandler* target = NULL,
									float minValue = 0.0f,
									float maxValue = 1.0f);
	virtual						~DualSlider();

	// BView interface
	virtual	void				Draw(BRect updateRect);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* dragMessage);

	virtual	BSize				MinSize();
	virtual	BSize				MaxSize();
	virtual	BSize				PreferredSize();

	// DualSlider
			void				SetLabel(const char* label);

			void				SetValues(float min, float max);

			void				SetMinValue(float value);
	inline	float				MinValue() const
									{ return fMinValue; }

			void				SetMaxValue(float value);
	inline	float				MaxValue() const
									{ return fMaxValue; }

			void				SetEnabled(bool enable);
			bool				IsEnabled() const;

			void				SetMinEnabled(bool enable, 
									bool sendMessage = false);
			bool				IsMinEnabled() const;

			void				SetMaxEnabled(bool enable);
			bool				IsMaxEnabled() const;

			void				SetPressureControlTip(const char* text);

private:
			void				_Invoke(BMessage* message);
			void				_InvalidateSlider();
			void				_StrokeRect(BRect frame, rgb_color leftTop,
									rgb_color rightBottom);

			float				_ValueFor(BPoint where) const;
			BRect				_BarFrame() const;
			BRect				_SliderFrame() const;
			BRect				_PressureBoxFrame() const;
			float				_LabelHeight() const;

private:
			float				fMinValue;
			float				fMaxValue;
			float				fLastFactor;

			uint32				fFlags;

			BMessage*			fValueMessage;
			BMessage*			fControlMessage;
			BHandler*			fTarget;

			BString				fLabel;
			BString				fPressureControlTip;
};

#endif // DUAL_SLIDER_H
