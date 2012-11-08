/*
 * Copyright 2001-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef	FONT_POPUP_H
#define FONT_POPUP_H

#include <MenuItem.h>

#include "LabelPopup.h"

// Menu item class displaying using a custom font as a label
class FontMenuItem : public BMenuItem {
public:
								FontMenuItem(const char* label,
									font_family fontFamily,
									font_style fontStyle,
									BMessage* model = NULL);
	virtual						~FontMenuItem();

	virtual	void				GetContentSize(float *width, float *height);
	virtual	void				DrawContent();
	virtual	status_t			Invoke(BMessage* messge = NULL);

			void				GetFamilyAndStyle(font_family* family,
									font_style* style) const;

private:
		BFont*					fFont;

		typedef BMenuItem _inherited;
};

// LabelPopup class that builds the label of
// the super item from items that have sub menus
class FontPopup : public LabelPopup {
public:
								FontPopup(const char* label, bool subMenus,
									bool asLabel = false);

	virtual	void				MessageReceived(BMessage* message);

								// FontPopup
			void				SetFamilyAndStyle(const char* family,
									const char* style);

private:
			bool				fSubMenus;
};


#endif // FONT_POPUP_H
