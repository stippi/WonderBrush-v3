/*
 * Copyright 2006, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef DOCUMENT_SAVER_H
#define DOCUMENT_SAVER_H

#include <SupportDefs.h>

#include "Document.h"

class DocumentSaver {
 public:
								DocumentSaver();
	virtual						~DocumentSaver();

	virtual	status_t			Save(const DocumentRef& document) = 0;
};

#endif // DOCUMENT_SAVER_H
