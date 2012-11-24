/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef IMPORTER_H
#define IMPORTER_H

#include <SupportDefs.h>

class Document;

class Importer {
public:
								Importer();
	virtual						~Importer();

	virtual	status_t			Init(Document* document);
};

#endif // IMPORTER_H
