/*
 * Copyright 2006, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef BITMAP_SET_SAVER_H
#define BITMAP_SET_SAVER_H

#include "FileSaver.h"

class BitmapSetSaver : public FileSaver {
 public:
								BitmapSetSaver(const entry_ref& ref);
	virtual						~BitmapSetSaver();

	virtual	status_t			Save(const DocumentRef& document);
};

#endif // BITMAP_SET_SAVER_H
