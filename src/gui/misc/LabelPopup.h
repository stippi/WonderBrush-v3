/*
 * Copyright 2001-2009, Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2001-2009, Ingo Weinhold <ingo_weinhold@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef LABEL_POPUP_H
#define LABEL_POPUP_H

#include <MenuField.h>

class LabelPopup : public BMenuField {
public:
								LabelPopup(const char* label,
									BMenu* menu = NULL, bool asLabel = false);
	virtual						~LabelPopup();

	// LabelPopup
	virtual	void				RefreshItemLabel();
};

#endif // LABEL_POPUP_H
