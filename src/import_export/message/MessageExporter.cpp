/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "MessageExporter.h"

#include <ByteOrder.h>
#include <DataIO.h>
#include <Message.h>
#include <TypeConstants.h>

// constructor
MessageExporter::MessageExporter()
{
}

// destructor
MessageExporter::~MessageExporter()
{
}

// Export
status_t
MessageExporter::Export(const LayerSnapshot* rootSnapshot, BPositionIO* stream)
{
	status_t ret = B_OK;
	BMessage archive;

	// Prepend the magic number to the file which later tells us that this file
	// is one of us
	if (ret == B_OK) {
		ssize_t size = sizeof(uint32);
		uint32 magic = B_HOST_TO_BENDIAN_INT32('WBI2');
		ssize_t written = stream->Write(&magic, size);
		if (written != size) {
			if (written < 0)
				ret = (status_t)written;
			else
				ret = B_IO_ERROR;
		}
	}

	if (ret == B_OK)
		ret = archive.Flatten(stream);

	return ret;
}

// MIMEType
const char*
MessageExporter::MIMEType()
{
	return "image/x-wonderbrush-2";
}

// #pragma mark -

