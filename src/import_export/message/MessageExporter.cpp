/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "MessageExporter.h"

#include <ByteOrder.h>
#include <DataIO.h>
#include <Message.h>
#include <TypeConstants.h>

#include "LayerSnapshot.h"
#include "ObjectSnapshot.h"
#include "RectSnapshot.h"
#include "ShapeSnapshot.h"
#include "TextSnapshot.h"

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

	if (ret == B_OK) {
		BMessage archive;
		ret = _ExportRoot(rootSnapshot, archive);

		if (ret == B_OK)
			ret = archive.Flatten(stream);
	}

	return ret;
}

// MIMEType
const char*
MessageExporter::MIMEType()
{
	return "image/x-wonderbrush-2";
}

// #pragma mark -

// _ExportRoot
status_t
MessageExporter::_ExportRoot(const LayerSnapshot* rootSnapshot,
	BMessage& archive)
{
	status_t ret = _ExportLayer(rootSnapshot, archive);

	archive.PrintToStream();

	return ret;
}

// _ExportLayer
status_t
MessageExporter::_ExportLayer(const LayerSnapshot* layer,
	BMessage& archive)
{
	status_t ret = B_OK;

	BMessage layerArchive;

	int32 count = layer->CountObjects();
	for (int32 i = 0; i < count; i++) {
		const ObjectSnapshot* object = layer->ObjectAtFast(i);
		const LayerSnapshot* layer = dynamic_cast<const LayerSnapshot*>(object);
		if (layer != NULL)
			ret = _ExportLayer(layer, layerArchive);
		else
			ret = _ExportObject(object, layerArchive);
		if (ret != B_OK)
			break;
	}

	if (ret == B_OK)
		ret = archive.AddMessage("layer", &layerArchive);

	return ret;
}

// _ExportObject
status_t
MessageExporter::_ExportObject(const ObjectSnapshot* object,
	BMessage& archive)
{
	BMessage objectArchive;
	status_t ret = objectArchive.AddString("name", object->Name());
	if (ret != B_OK)
		return ret;

	// Determine object type
	const RectSnapshot* rect = dynamic_cast<const RectSnapshot*>(object);
	if (rect != NULL) {
		ret = _ExportRectangle(rect, objectArchive);
		if (ret == B_OK)
			ret = archive.AddMessage("rectangle", &objectArchive);
		return ret;
	}

	const ShapeSnapshot* shape = dynamic_cast<const ShapeSnapshot*>(object);
	if (shape != NULL) {
		ret = _ExportShape(shape, objectArchive);
		if (ret == B_OK)
			ret = archive.AddMessage("shape", &objectArchive);
		return ret;
	}
	
	const TextSnapshot* text = dynamic_cast<const TextSnapshot*>(object);
	if (text != NULL) {
		ret = _ExportText(text, objectArchive);
		if (ret == B_OK)
			ret = archive.AddMessage("text", &objectArchive);
		return ret;
	}
	
	return B_OK;
}

// _ExportRectangle
status_t
MessageExporter::_ExportRectangle(const RectSnapshot* rect,
	BMessage& archive)
{
	status_t ret = B_OK;
	if (ret == B_OK)
		ret = archive.AddRect("area", rect->Area());
	if (ret == B_OK)
		ret = archive.AddDouble("radius", rect->RoundCornerRadius());
	return ret;
}

// _ExportShape
status_t
MessageExporter::_ExportShape(const ShapeSnapshot* shape,
	BMessage& archive)
{
	return B_OK;
}

// _ExportText
status_t
MessageExporter::_ExportText(const TextSnapshot* text,
	BMessage& archive)
{
	return B_OK;
}



