/*
 * Copyright 2007-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */


#include "NativeSaver.h"

#include <stdio.h>
#include <string.h>

#include "MessageExporter.h"


NativeSaver::NativeSaver(const entry_ref& ref)
	:
	SimpleFileSaver(new MessageExporter(), ref),
	fAttrSaver(ref, "BEOS:L:STD_ICON")
{
}


NativeSaver::~NativeSaver()
{
}


status_t
NativeSaver::Save(const DocumentRef& document)
{
	status_t ret = SimpleFileSaver::Save(document);
	if (ret != B_OK) {
		fprintf(stderr, "Error saving icon: %s\n", strerror(ret));
		return ret;
	}

	WaitForExportThread();

	ret = fAttrSaver.Save(document);
	if (ret != B_OK) {
		fprintf(stderr, "Error saving icon attribute: %s\n", strerror(ret));
		return ret;
	}

	return B_OK;
}

