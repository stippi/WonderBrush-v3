/*
 * Copyright 2014, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef BITMAP_IMPORTER_H
#define BITMAP_IMPORTER_H

#include "Document.h"

class BMessage;
class BPositionIO;
class BaseObject;
class BoundedObject;
class Object;
class Styleable;

class BitmapImporter {
public:
								BitmapImporter(const DocumentRef& document);
	virtual						~BitmapImporter();

			status_t			Import(BPositionIO& stream) const;
private:
			DocumentRef			fDocument;
};

#endif // BITMAP_IMPORTER_H
