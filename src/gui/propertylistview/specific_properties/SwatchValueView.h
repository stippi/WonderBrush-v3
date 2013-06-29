/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef SWATCH_VALUE_VIEW_H
#define SWATCH_VALUE_VIEW_H

#include "SwatchView.h"

class SwatchValueView : public SwatchView {
public:
								SwatchValueView(const char* name,
									BMessage* message, BHandler* target,
									rgb_color color, float width = 26.0,
									float height = 26.0);
	virtual						~SwatchValueView();

	// BView interface
	virtual	void				MakeFocus(bool focused);

	virtual	void				PlatformDraw(PlatformDrawContext& drawContext);

	virtual	void				MouseDown(BPoint where);

};

#endif // SWATCH_VALUE_VIEW_H


