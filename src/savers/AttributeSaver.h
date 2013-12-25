/*
 * Copyright 2006, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef ATTRIBUTE_SAVER_H
#define ATTRIBUTE_SAVER_H

#include <Entry.h>
#include <String.h>

#include "DocumentSaver.h"

class AttributeSaver : public DocumentSaver {
public:
								AttributeSaver(const entry_ref& ref,
									const char* attrName);
	virtual						~AttributeSaver();

	virtual	status_t			Save(const DocumentRef& document);

protected:
			entry_ref			fRef;
			BString				fAttrName;
};

#endif // ATTRIBUTE_SAVER_H
