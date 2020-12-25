/*
 * Copyright 2002-2020 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "DualSlider.h"

#include <stdio.h>

#include <Bitmap.h>
#include <ControlLook.h>
#include <LayoutUtils.h>
#include <GradientLinear.h>
#include <Looper.h>
#include <Message.h>
#include <Region.h>
#include <Shape.h>
#include <Window.h>

#include "support.h"
#include "support_ui.h"

enum {
	MIN_ENABLED			= 0x01,
	MAX_ENABLED			= 0x02,
	DRAGGING_MIN		= 0x04,
	DRAGGING_MAX		= 0x08,
	PRESSURE_INSIDE		= 0x10,
	PRESSURE_PRESSED	= 0x20,
	TILT_INSIDE			= 0x40,
	DISABLED			= 0x80,
};

#define LABEL_SPACING 2.0

// constructor
DualSlider::DualSlider(const char* name, const char* label,
		BMessage* valueMessage, BMessage* controlMessage, BHandler* target,
		float minValue, float maxValue)
	: BView(name, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE)
	, fMinValue(minValue)
	, fMaxValue(maxValue)
	, fLastFactor(0.0)
	, fFlags(MIN_ENABLED | MAX_ENABLED)
	, fValueMessage(valueMessage)
	, fControlMessage(controlMessage)
	, fTarget(target)
	, fLabel(label)
	, fPressureControlTip("Enables control by pen pressure.")
{
	SetViewColor(B_TRANSPARENT_COLOR);
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

// destructor
DualSlider::~DualSlider()
{
	delete fValueMessage;
	delete fControlMessage;
}

// #pragma mark -

// Draw
void
DualSlider::Draw(BRect updateRect)
{
	rgb_color background;
	rgb_color shadow;
	rgb_color light;
	rgb_color blue;
	rgb_color softLight;
	rgb_color darkShadow;
	rgb_color darkestShadow;
	rgb_color softBlack;
	rgb_color lightShadow;
	rgb_color black;
	if (IsEnabled()) {
		background = ui_color(B_PANEL_BACKGROUND_COLOR);
		shadow = tint_color(background, B_DARKEN_2_TINT);
		light = tint_color(background, B_LIGHTEN_MAX_TINT);
		blue = ui_color(B_KEYBOARD_NAVIGATION_COLOR);
		softLight = tint_color(background, B_LIGHTEN_2_TINT);
		darkShadow = tint_color(background, B_DARKEN_3_TINT);
		darkestShadow = tint_color(background, 1.7);
		softBlack = tint_color(background, B_DARKEN_4_TINT);
		lightShadow = tint_color(background, B_DARKEN_1_TINT);
		black = tint_color(background, B_DARKEN_MAX_TINT);
	} else {
		background = ui_color(B_PANEL_BACKGROUND_COLOR);
		shadow = tint_color(background, B_DARKEN_1_TINT);
		light = tint_color(background, B_LIGHTEN_2_TINT);
		blue = tint_color(ui_color(B_KEYBOARD_NAVIGATION_COLOR),
						  B_LIGHTEN_2_TINT);
		softLight = background;//tint_color(background, B_LIGHTEN_2_TINT);
		darkShadow = tint_color(background, B_DARKEN_1_TINT);
		softBlack = tint_color(background, B_DARKEN_2_TINT);
		darkestShadow = softBlack;
		lightShadow = background;//tint_color(background, B_DARKEN_1_TINT);
		black = tint_color(background, B_DARKEN_3_TINT);
	}
	uint32 flags = 0;
	float uiScale = ui_scale();
	if (!IsEnabled())
		flags |= BControlLook::B_DISABLED;
	BRect r(Bounds());
	// label
	float labelHeight = _LabelHeight(uiScale);
	if (labelHeight > 0.0) {
		r.bottom = r.top + labelHeight;
		FillRect(r, B_SOLID_LOW);
//		r.left += 5.0;
		font_height fh;
		GetFontHeight(&fh);
		SetHighColor(black);
		DrawString(fLabel.String(), BPoint(r.left, r.top + fh.ascent + 1.0));
	}
	// pressure check mark
	r = _PressureBoxFrame(uiScale);
	if (updateRect.Intersects(r) && fControlMessage) {
		uint32 pressureFlags = flags;
		if ((fFlags & PRESSURE_PRESSED) != 0
			&& (fFlags & PRESSURE_INSIDE) != 0) {
			pressureFlags |= BControlLook::B_CLICKED;
		}
		if (IsMinEnabled())
			pressureFlags |= BControlLook::B_ACTIVATED;
		PushState();
		be_control_look->DrawCheckBox(this, r, updateRect, background,
			pressureFlags);
		PopState();
	}
	// slider background
	r = _SliderFrame(uiScale);
	BRect barFrame = _BarFrame(uiScale);
	BRect top(r.left, r.top, r.right, barFrame.top - 1.0);
	BRect left(r.left, barFrame.top,
			   barFrame.left - 1.0, barFrame.bottom);
	BRect right(barFrame.right + 1.0, barFrame.top,
				r.right, barFrame.bottom);
	BRect bottom(r.left, barFrame.bottom + 1.0,
				 r.right, r.bottom);
	FillRect(top, B_SOLID_LOW);
	FillRect(left, B_SOLID_LOW);
	FillRect(right, B_SOLID_LOW);
	FillRect(bottom, B_SOLID_LOW);
	// slider bar
	BRect innerBarFrame(barFrame.InsetByCopy(1.0f, 0.0f));
	float minPos = barFrame.left;
	float maxPos = floorf(innerBarFrame.left + innerBarFrame.Width()
		* fMaxValue + 0.5);
	if (IsMinEnabled()) {
		minPos = floorf(innerBarFrame.left + innerBarFrame.Width()
			* fMinValue + 0.5);
		if (minPos > barFrame.left) {
			BRegion clipping(BRect(barFrame.left, barFrame.top, minPos,
					barFrame.bottom));
			ConstrainClippingRegion(&clipping);
			PushState();
			be_control_look->DrawSliderBar(this, barFrame, updateRect, background,
				be_control_look->SliderBarColor(background), flags, B_HORIZONTAL);
			PopState();
		}
		if (maxPos < barFrame.right) {
			BRegion clipping(BRect(maxPos, barFrame.top, barFrame.right,
					barFrame.bottom));
			ConstrainClippingRegion(&clipping);
			PushState();
			be_control_look->DrawSliderBar(this, barFrame, updateRect, background,
				be_control_look->SliderBarColor(background), flags, B_HORIZONTAL);
			PopState();
		}
		BRegion clipping(BRect(min_c(minPos, maxPos), barFrame.top,
			max_c(minPos, maxPos), barFrame.bottom));
		ConstrainClippingRegion(&clipping);
		PushState();
		be_control_look->DrawSliderBar(this, barFrame, updateRect, background,
			darkShadow, flags, B_HORIZONTAL);
		PopState();
		ConstrainClippingRegion(NULL);
	} else {
		PushState();
		be_control_look->DrawSliderBar(this, barFrame, updateRect, background,
			be_control_look->SliderBarColor(background), flags, B_HORIZONTAL);
		PopState();
	}

	barFrame.InsetBy(1.0, 1.0);
	barFrame.left += 2;
	barFrame.top++;

	// arrows
	BRect arrowRect;
	if (IsMinEnabled()) {
		arrowRect.Set(
			minPos - 5.0 * uiScale,
			barFrame.top - 6.0 * uiScale,
			minPos + 6.0 * uiScale,
			barFrame.top + 1);
		_DrawSliderTriangleDownward(this, arrowRect, updateRect, background,
			background, flags);
	}
	arrowRect.Set(
		maxPos - 6.0 * uiScale,
		barFrame.bottom - 1.0,
		maxPos + 7.0 * uiScale,
		barFrame.bottom + 7.0 * uiScale);
	be_control_look->DrawSliderTriangle(this, arrowRect, updateRect,
		background, flags, B_HORIZONTAL);
}

// MouseDown
void
DualSlider::MouseDown(BPoint where)
{
	if (!IsEnabled())
		return;

	float uiScale = ui_scale();
	BRect r(_SliderFrame(uiScale));
	BRect minFrame(r);
	if (IsMaxEnabled() && IsMinEnabled())
		minFrame.bottom = r.top + floorf(r.Height() / 2.0);
	BRect maxFrame(r);
	if (IsMaxEnabled() && IsMinEnabled())
		maxFrame.top = minFrame.bottom + 1.0;
	float value = _ValueFor(where);
	if (IsMinEnabled() && minFrame.Contains(where)) {
		fFlags |= DRAGGING_MIN;
		fFlags &= ~DRAGGING_MAX;
		fLastFactor = fMaxValue / fMinValue;
		SetMinValue(value);
		if (fLastFactor < 1.0)
			SetMaxValue(fLastFactor * fMinValue);
		SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
	} else if (IsMaxEnabled() && maxFrame.Contains(where)) {
		fFlags |= DRAGGING_MAX;
		fFlags &= ~DRAGGING_MIN;
		fLastFactor = fMinValue / fMaxValue;
		SetMaxValue(value);
		if (fLastFactor < 1.0)
			SetMinValue(fLastFactor * fMaxValue);
		SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
	}
	r = _PressureBoxFrame(uiScale);
	if (fControlMessage && r.Contains(where)) {
		fFlags |= PRESSURE_PRESSED;
		Invalidate(r);
	}
}

// MouseUp
void
DualSlider::MouseUp(BPoint where)
{
	if ((fFlags & PRESSURE_PRESSED) != 0) {
		BRect r(_PressureBoxFrame(ui_scale()));
		if (r.Contains(where))
			SetMinEnabled(!IsMinEnabled(), true);
		Invalidate(r);
	}

	fFlags &= ~PRESSURE_PRESSED;
	fFlags &= ~DRAGGING_MIN;
	fFlags &= ~DRAGGING_MAX;
}

// MouseMoved
void
DualSlider::MouseMoved(BPoint where, uint32 transit,
					   const BMessage* dragMessage)
{
	if (IsEnabled()) {
		float value = _ValueFor(where);
		if (fFlags & DRAGGING_MIN) {
			SetMinValue(value);
			if (fLastFactor < 1.0)
				SetMaxValue(fLastFactor * fMinValue);
		} else if (fFlags & DRAGGING_MAX) {
			SetMaxValue(value);
			if (fLastFactor < 1.0)
				SetMinValue(fLastFactor * fMaxValue);
		}
	}
	BRect r(_PressureBoxFrame(ui_scale()));
	if (r.Contains(where)) {
		if ((fFlags & PRESSURE_INSIDE) == 0) {
			fFlags |= PRESSURE_INSIDE;
			Invalidate(r);
			SetToolTip((BToolTip*)NULL);
			SetToolTip(fPressureControlTip.String());
		}
	} else {
		if ((fFlags & PRESSURE_INSIDE) != 0) {
			fFlags &= ~PRESSURE_INSIDE;
			Invalidate(r);
			HideToolTip();
			SetToolTip((BToolTip*)NULL);
		}
	}
}

// MinSize
BSize
DualSlider::MinSize()
{
	float uiScale = ui_scale();
	BSize size(0.0, 18.0 * uiScale);
	if (fLabel.Length() > 0) {
		font_height fh;
		GetFontHeight(&fh);
		size.height += ceilf(fh.ascent) + ceilf(fh.descent) + LABEL_SPACING;
		size.width = StringWidth(fLabel.String());
		if (fControlMessage != NULL)
			size.width += 20.0 * uiScale;
	}
	if (size.width == 0.0)
		size.width = 60.0 * uiScale;

	return BLayoutUtils::ComposeSize(ExplicitMinSize(), size);
}

// MaxSize
BSize
DualSlider::MaxSize()
{
	BSize size(MinSize());
	size.width = B_SIZE_UNLIMITED;
	
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(), size);
}

// PreferredSize
BSize
DualSlider::PreferredSize()
{
	BSize size(MinSize());
	
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(), size);
}

// #pragma mark -

// SetValues
void
DualSlider::SetLabel(const char* label)
{
	if (label) {
		fLabel.SetTo(label);
		Invalidate();
	}
}

// SetValues
void
DualSlider::SetValues(float min, float max)
{
	SetMinValue(min);
	SetMaxValue(max);
}

// SetMinValue
void
DualSlider::SetMinValue(float value)
{
	if (value < 0.0)
		value = 0.0;
	if (value > 1.0)
		value = 1.0;
	if (value != fMinValue) {
		fMinValue = value;
		_InvalidateSlider();
		_Invoke(fValueMessage);
	}
}

// SetMaxValue
void
DualSlider::SetMaxValue(float value)
{
	if (value < 0.0)
		value = 0.0;
	if (value > 1.0)
		value = 1.0;
	if (value != fMaxValue) {
		fMaxValue = value;
		_InvalidateSlider();
		_Invoke(fValueMessage);
	}
}

// SetEnabled
void
DualSlider::SetEnabled(bool enable)
{
	if (enable != IsEnabled()) {
		if (enable)
			fFlags &= ~DISABLED;
		else
			fFlags |= DISABLED;
		Invalidate();
	}
}

// IsEnabled
bool
DualSlider::IsEnabled() const
{
	return !(fFlags & DISABLED);
}

// SetMinEnabled
void
DualSlider::SetMinEnabled(bool enable, bool sendMessage)
{
	float uiScale = ui_scale();
	if (enable) {
		if (!(fFlags & MIN_ENABLED)) {
			fFlags |= MIN_ENABLED;
			Invalidate(_PressureBoxFrame(uiScale));
			_InvalidateSlider();
			if (sendMessage)
				_Invoke(fControlMessage);
		}
	} else {
		if (fFlags & MIN_ENABLED) {
			fFlags &= ~MIN_ENABLED;
			Invalidate(_PressureBoxFrame(uiScale));
			_InvalidateSlider();
			if (sendMessage)
				_Invoke(fControlMessage);
		}
	}
}

// IsMinEnabled
bool
DualSlider::IsMinEnabled() const
{
	return fFlags & MIN_ENABLED;
}

// SetMaxEnabled
void
DualSlider::SetMaxEnabled(bool enable)
{
	if (enable) {
		if (!(fFlags & MAX_ENABLED)) {
			fFlags |= MAX_ENABLED;
			_InvalidateSlider();
		}
	} else {
		if (fFlags & MAX_ENABLED) {
			fFlags &= ~MAX_ENABLED;
			_InvalidateSlider();
		}
	}
}

// IsMaxEnabled
bool
DualSlider::IsMaxEnabled() const
{
	return fFlags & MAX_ENABLED;
}

// SetPressureControlTip
void
DualSlider::SetPressureControlTip(const char* text)
{
	fPressureControlTip.SetTo(text);
}

// #pragma mark -

// _Invoke
void
DualSlider::_Invoke(BMessage* fromMessage)
{
	if (fromMessage == NULL)
		fromMessage = fValueMessage;
	if (fromMessage != NULL) {
		BHandler* target = fTarget != NULL ? fTarget : Window();
		BLooper* looper;
		if (target != NULL && (looper = target->Looper())) {
			BMessage message(*fromMessage);
			message.AddPointer("be:source", (void*)this);
			message.AddInt64("be:when", system_time());
			if (fromMessage == fValueMessage) {
				message.AddFloat("min value", fMinValue);
				message.AddFloat("max value", fMaxValue);
			}
			if (fromMessage == fControlMessage)
				message.AddInt32("be:value", (int32)IsMinEnabled());
			looper->PostMessage(&message, target);
		}
	}
}

// _InvalidateSlider
void
DualSlider::_InvalidateSlider()
{
	Invalidate(_SliderFrame(ui_scale()));
}

void
DualSlider::_DrawSliderTriangleDownward(BView* view, BRect& rect,
	const BRect& updateRect, const rgb_color& base, const rgb_color& fill,
	uint32 flags) const
{
	if (!rect.IsValid() || !rect.Intersects(updateRect))
		return;

	// figure out frame color
	rgb_color frameLightColor;
	rgb_color frameShadowColor;
	rgb_color shadowColor = (rgb_color){ 0, 0, 0, 60 };

	float topTint = 0.49;
	float middleTint1 = 0.62;
	float middleTint2 = 0.76;
	float bottomTint = 0.90;

	if (flags & BControlLook::B_DISABLED) {
		topTint = (topTint + B_NO_TINT) / 2;
		middleTint1 = (middleTint1 + B_NO_TINT) / 2;
		middleTint2 = (middleTint2 + B_NO_TINT) / 2;
		bottomTint = (bottomTint + B_NO_TINT) / 2;
	} else if (flags & BControlLook::B_HOVER) {
		static const float kHoverTintFactor = 0.85;
		topTint *= kHoverTintFactor;
		middleTint1 *= kHoverTintFactor;
		middleTint2 *= kHoverTintFactor;
		bottomTint *= kHoverTintFactor;
	}

	if (flags & BControlLook::B_FOCUSED) {
		// focused
		frameLightColor = ui_color(B_KEYBOARD_NAVIGATION_COLOR);
		frameShadowColor = frameLightColor;
	} else {
		// figure out the tints to be used
		float frameLightTint;
		float frameShadowTint;

		if (flags & BControlLook::B_DISABLED) {
			frameLightTint = 1.30;
			frameShadowTint = 1.35;
			shadowColor.alpha = 30;
		} else {
			frameLightTint = 1.6;
			frameShadowTint = 1.65;
		}

		frameLightColor = tint_color(base, frameLightTint);
		frameShadowColor = tint_color(base, frameShadowTint);
	}

	// make room for the shadow
	rect.right--;
	rect.bottom--;

	uint32 viewFlags = view->Flags();
	view->SetFlags(viewFlags | B_SUBPIXEL_PRECISE);
	view->SetLineMode(B_ROUND_CAP, B_ROUND_JOIN);

	float center = (rect.left + rect.right) / 2;

	BShape shape;
	shape.MoveTo(BPoint(rect.left + 0.5, rect.top + 0.5));
	shape.LineTo(BPoint(rect.right + 0.5, rect.top + 0.5));
	shape.LineTo(BPoint(rect.right + 0.5, rect.top + 1 + 0.5));
	shape.LineTo(BPoint(center + 0.5, rect.bottom + 0.5));
	shape.LineTo(BPoint(rect.left + 0.5, rect.top + 1 + 0.5));
	shape.Close();

	view->MovePenTo(BPoint(0.5, 0.5));

	view->SetDrawingMode(B_OP_ALPHA);
	view->SetHighColor(shadowColor);
	view->StrokeShape(&shape);

	view->MovePenTo(B_ORIGIN);

	view->SetDrawingMode(B_OP_OVER);
	view->SetHighColor(frameLightColor);
	view->StrokeShape(&shape);

	rect.InsetBy(1, 1);
	shape.Clear();
	shape.MoveTo(BPoint(rect.left, rect.top));
	shape.LineTo(BPoint(rect.right + 1, rect.top));
	shape.LineTo(BPoint(center + 0.5, rect.bottom + 1));
	shape.Close();

	BGradientLinear gradient;
	if (flags & BControlLook::B_DISABLED) {
		_MakeGradient(gradient, rect, fill, topTint, bottomTint);
	} else {
		_MakeGlossyGradient(gradient, rect, fill, topTint, middleTint1,
			middleTint2, bottomTint);
	}

	view->FillShape(&shape, gradient);

	view->SetFlags(viewFlags);
}

void
DualSlider::_MakeGradient(BGradientLinear& gradient, const BRect& rect,
	const rgb_color& base, float topTint, float bottomTint,
	enum orientation orientation) const
{
	gradient.AddColor(tint_color(base, topTint), 0);
	gradient.AddColor(tint_color(base, bottomTint), 255);
	gradient.SetStart(rect.LeftTop());
	if (orientation == B_HORIZONTAL)
		gradient.SetEnd(rect.LeftBottom());
	else
		gradient.SetEnd(rect.RightTop());
}


void
DualSlider::_MakeGlossyGradient(BGradientLinear& gradient, const BRect& rect,
	const rgb_color& base, float topTint, float middle1Tint,
	float middle2Tint, float bottomTint,
	enum orientation orientation) const
{
	gradient.AddColor(tint_color(base, topTint), 0);
	gradient.AddColor(tint_color(base, middle1Tint), 132);
	gradient.AddColor(tint_color(base, middle2Tint), 136);
	gradient.AddColor(tint_color(base, bottomTint), 255);
	gradient.SetStart(rect.LeftTop());
	if (orientation == B_HORIZONTAL)
		gradient.SetEnd(rect.LeftBottom());
	else
		gradient.SetEnd(rect.RightTop());
}

// _ValueFor
float
DualSlider::_ValueFor(BPoint where) const
{
	BRect r(_BarFrame(ui_scale()));
	return (where.x - r.left) / r.Width();
}


// _BarFrame
BRect
DualSlider::_BarFrame(float uiScale) const
{
	BRect r(_SliderFrame(uiScale));
	r.InsetBy(6.0 * uiScale, 5.0 * uiScale);
	r.bottom = r.top + 6.0 * uiScale;
	return r;
}

// _SliderFrame
BRect
DualSlider::_SliderFrame(float uiScale) const
{
	BRect r(Bounds());
	r.top += _LabelHeight(uiScale);
	return r;
}

// _PressureBoxFrame
BRect
DualSlider::_PressureBoxFrame(float uiScale) const
{
	BRect r(Bounds());
	r.right -= 6.0 * uiScale;
	r.left = r.right - 12.0 * uiScale;
	r.bottom = r.top + 12.0 * uiScale;
	return r;
}

// _LabelHeight
float
DualSlider::_LabelHeight(float uiScale) const
{
	float height = 0.0;
	if (fLabel.CountChars() > 0) {
		font_height fh;
		GetFontHeight(&fh);
		height = ceilf(fh.ascent + fh.descent) + LABEL_SPACING * uiScale;
	}
	return height;
}

