/*
 * Copyright 2006-2020, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef SUPPORT_UI_H
#define SUPPORT_UI_H

#include <GraphicsDefs.h>
#include <Rect.h>
//#include <agg_math_stroke.h>

#include "BuildSupport.h"

class BBitmap;
class BDataIO;
class BMessage;
class BPositionIO;
class BString;
class BTextControl;
class BView;
class BWindow;

class PlatformDrawContext;


status_t store_color_in_message(BMessage* message, rgb_color color);

status_t restore_color_from_message(const BMessage* message, rgb_color& color, int32 index = 0);

BMessage make_color_drop_message(rgb_color color, BBitmap* bitmap);

void make_sure_frame_is_on_screen(BRect& frame, BWindow* window);

void print_modifiers();

//agg::line_cap_e convert_cap_mode(uint32 mode);
//agg::line_join_e convert_join_mode(uint32 mode);

const char* string_for_color_space(color_space format);
void print_color_space(color_space format);


// Those are already defined in newer versions of BeOS
#if !defined(B_BEOS_VERSION_DANO) && !defined(__HAIKU__) \
	&& !defined(WONDERBRUSH_PLATFORM_QT)

// rgb_color == rgb_color
static inline bool
operator==(const rgb_color& a, const rgb_color& b)
{
	return a.red == b.red
			&& a.green == b.green
			&& a.blue == b.blue
			&& a.alpha == b.alpha;
}

// rgb_color != rgb_color
static inline bool
operator!=(const rgb_color& a, const rgb_color& b)
{
	return !(a == b);
}

#endif // B_BEOS_VERSION <= ...

// produces values between 1 and maxValue that increase exponentially for
// linear values in the same range
double from_linear(double value, double maxValue);

// produces values between 1 and maxValue that increase linearily for
// exponentially increasing values in the same range
double to_linear(double value, double maxValue);

void set_text_control_float_value(BTextControl* control, float value);
float get_text_control_float_value(BTextControl* control);

// platform dependent

// looper of view must be locked!
void stroke_frame(PlatformDrawContext& drawContext, BRect frame,
	rgb_color left, rgb_color top, rgb_color right, rgb_color bottom);


float ui_scale();
int icon_size();
BSize scaled_ui_size(BSize size);

#endif // SUPPORT_UI_H
