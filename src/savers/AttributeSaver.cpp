/*
 * Copyright 2006, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "AttributeSaver.h"

#include <Node.h>

#include "Document.h"

// constructor
AttributeSaver::AttributeSaver(const entry_ref& ref, const char* attrName)
	: fRef(ref)
	, fAttrName(attrName)
{
}

// destructor
AttributeSaver::~AttributeSaver()
{
}

// Save
status_t
AttributeSaver::Save(const DocumentRef& document)
{
	// TODO: Render bitmap at icon resolution and store in attribute
	return B_OK;
}

