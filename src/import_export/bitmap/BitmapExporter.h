/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef BITMAP_EXPORTER_H
#define BITMAP_EXPORTER_H

#include "Exporter.h"

class BitmapExporter : public Exporter {
public:
								BitmapExporter(const BRect& bounds);
								BitmapExporter(uint32 width, uint32 height);
	virtual						~BitmapExporter();

	virtual	status_t			Export(const DocumentRef& document,
									BPositionIO* stream);

	virtual	const char*			MIMEType();

private:
			uint32				fFormat;
			uint32				fWidth;
			uint32				fHeight;
};

#endif // BITMAP_EXPORTER_H
