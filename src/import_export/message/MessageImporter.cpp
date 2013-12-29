/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "MessageImporter.h"

#include <ByteOrder.h>
#include <DataIO.h>
#include <Message.h>
#include <TypeConstants.h>

#include "BoundedObject.h"
#include "Brush.h"
#include "BrushStroke.h"
#include "CharacterStyle.h"
#include "Color.h"
#include "ColorShade.h"
#include "Document.h"
#include "Filter.h"
#include "FilterDropShadow.h"
#include "FilterSaturation.h"
#include "Image.h"
#include "Layer.h"
#include "Object.h"
#include "Paint.h"
#include "StrokeProperties.h"
#include "Style.h"
#include "Styleable.h"
#include "StyleRun.h"
#include "Shape.h"
#include "Rect.h"
#include "Text.h"

static const char* kType = "type";

// constructor
MessageImporter::MessageImporter(const DocumentRef& document)
	: fDocument(document)
{
}

// destructor
MessageImporter::~MessageImporter()
{
}

// Import
status_t
MessageImporter::Import(BPositionIO& stream) const
{
	if (fDocument.Get() == NULL)
		return B_NO_INIT;

	uint32 magic = 0;
	ssize_t size = sizeof(magic);
	ssize_t read = stream.Read(&magic, size);
	if (read != size) {
		if (read < 0)
			return (status_t)read;
		else
			return B_IO_ERROR;
	}

	if (B_BENDIAN_TO_HOST_INT32(magic) != 'WBI2')
		return B_BAD_VALUE;

	BMessage archive;
	status_t ret = archive.Unflatten(&stream);
	if (ret != B_OK)
		return ret;

	return ImportDocument(archive);
}

// ImportDocument
status_t
MessageImporter::ImportDocument(const BMessage& archive) const
{
	BMessage resources;
	if (archive.FindMessage("resources", &resources) != B_OK) {
		fprintf(stderr, "MessageImporter::ImportDocument() "
			"- \"resources\" not found.\n");
		return B_ERROR;
	}

	status_t ret = ImportGlobalResources(resources);
	if (ret != B_OK)
		return ret;

	return ImportObjects(archive, fDocument->RootLayer());
}

// ImportGlobalResources
status_t
MessageImporter::ImportGlobalResources(const BMessage& archive) const
{
	return _ImportObjects<BaseObject, ResourceList>(
		archive, &fDocument->GlobalResources());
}

// ImportObjects
status_t
MessageImporter::ImportObjects(const BMessage& archive, Layer* layer) const
{
	return _ImportObjects<Object, Layer>(archive, layer);
}

// ImportObject
BaseObjectRef
MessageImporter::ImportObject(const BMessage& archive) const
{
	int32 resourceIndex;
	if (archive.FindInt32("resource", &resourceIndex) == B_OK) {
		return BaseObjectRef(
			fDocument->GlobalResources().ObjectAt(resourceIndex)
		);
	}
	
	BString type;
	if (archive.FindString(kType, &type) != B_OK) {
		fprintf(stderr, "MessageImporter::ImportObject() - "
			"Type string not found!\n");
		return BaseObjectRef();
	}

	// objects
	if (type == "BrushStroke")
		return ImportBrushStroke(archive);

	if (type == "FilterGaussianBlur")
		return ImportFilterGaussianBlur(archive);
		
	if (type == "FilterDropShadow")
		return ImportFilterDropShadow(archive);

	if (type == "FilterSaturation")
		return ImportFilterSaturation(archive);

	if (type == "Image")
		return ImportImage(archive);

	if (type == "Layer")
		return ImportLayer(archive);

	if (type == "Rect")
		return ImportRect(archive);

	if (type == "Shape")
		return ImportShape(archive);

	if (type == "Text")
		return ImportText(archive);

	// fills
	if (type == "Brush")
		return ImportBrush(archive);

	if (type == "Color")
		return ImportColor(archive);

	if (type == "ColorShade")
		return ImportColorShade(archive);

	if (type == "Gradient")
		return ImportGradient(archive);

	if (type == "Paint")
		return ImportPaint(archive);

	if (type == "Path")
		return ImportPath(archive);

	if (type == "StrokeProperties")
		return ImportStrokeProperties(archive);

	if (type == "Style")
		return ImportStyle(archive);

	// unhandled!
	fprintf(stderr, "MessageImporter::ImportObject() - "
		"Unkown object type: '%s'\n", type.String());

	return BaseObjectRef();
}

// ImportBrushStroke
BaseObjectRef
MessageImporter::ImportBrushStroke(const BMessage& archive) const
{
	return BaseObjectRef();
}

// ImportFilterGaussianBlur
BaseObjectRef
MessageImporter::ImportFilterGaussianBlur(const BMessage& archive) const
{
	return BaseObjectRef();
}

// ImportFilterDropShadow
BaseObjectRef
MessageImporter::ImportFilterDropShadow(const BMessage& archive) const
{
	return BaseObjectRef();
}

// ImportFilterSaturation
BaseObjectRef
MessageImporter::ImportFilterSaturation(const BMessage& archive) const
{
	return BaseObjectRef();
}

// ImportImage
BaseObjectRef
MessageImporter::ImportImage(const BMessage& archive) const
{
	return BaseObjectRef();
}

// ImportLayer
BaseObjectRef
MessageImporter::ImportLayer(const BMessage& archive) const
{
	Layer* layer = new(std::nothrow) Layer(fDocument->Bounds());
	if (layer != NULL)
		ImportObjects(archive, layer);
	return BaseObjectRef(layer, true);
}

// ImportRect
BaseObjectRef
MessageImporter::ImportRect(const BMessage& archive) const
{
	Rect* rect = new(std::nothrow) Rect();
	if (rect != NULL) {
		BRect area;
		if (archive.FindRect("area", &area) == B_OK)
			rect->SetArea(area);

		double roundCornerRadius;
		if (archive.FindDouble("radius", &roundCornerRadius) == B_OK)
			rect->SetRoundCornerRadius(roundCornerRadius);

		_RestoreStyleable(rect, archive);
	}
	return BaseObjectRef(rect, true);
}

// ImportShape
BaseObjectRef
MessageImporter::ImportShape(const BMessage& archive) const
{
	Shape* shape = new(std::nothrow) Shape();
	if (shape != NULL) {
		for (int32 i = 0;; i++) {
			BMessage pathArchive;
			if (archive.FindMessage("path", i, &pathArchive) != B_OK)
				break;
			BaseObjectRef pathRef = ImportObject(pathArchive);
			Path* path = dynamic_cast<Path*>(pathRef.Get());
			if (path != NULL)
				shape->AddPath(PathRef(path));
		}
		
		_RestoreStyleable(shape, archive);
	}
	return BaseObjectRef(shape, true);
}

// ImportText
BaseObjectRef
MessageImporter::ImportText(const BMessage& archive) const
{
	return BaseObjectRef();
}

// #pragma mark -

// ImportBrush
BaseObjectRef
MessageImporter::ImportBrush(const BMessage& archive) const
{
	return BaseObjectRef();
}

// ImportColor
BaseObjectRef
MessageImporter::ImportColor(const BMessage& archive) const
{
	return BaseObjectRef();
}

// ImportColorShade
BaseObjectRef
MessageImporter::ImportColorShade(const BMessage& archive) const
{
	return BaseObjectRef();
}

// ImportGradient
BaseObjectRef
MessageImporter::ImportGradient(const BMessage& archive) const
{
	return BaseObjectRef();
}

// ImportPaint
BaseObjectRef
MessageImporter::ImportPaint(const BMessage& archive) const
{
	return BaseObjectRef();
}

// ImportPath
BaseObjectRef
MessageImporter::ImportPath(const BMessage& archive) const
{
	Path* path = new(std::nothrow) Path(&archive);
	return BaseObjectRef(path, true);
}

// ImportStrokeProperties
BaseObjectRef
MessageImporter::ImportStrokeProperties(const BMessage& archive) const
{
	return BaseObjectRef();
}

// ImportStyle
BaseObjectRef
MessageImporter::ImportStyle(const BMessage& archive) const
{
	return BaseObjectRef();
}

// #pragma mark -

// ImportGlobalResources
template<class Type, class Container>
status_t
MessageImporter::_ImportObjects(const BMessage& archive,
	Container* container) const
{
	for (int32 i = 0;; i++) {
		BMessage objectArchive;
		status_t ret = archive.FindMessage("object", i, &objectArchive);
		if (ret != B_OK)
			break;

		BaseObjectRef object = ImportObject(objectArchive);
		Type* typedObject = dynamic_cast<Type*>(object.Get());
		if (typedObject == NULL)
			continue;

		if (!container->AddObject(typedObject))
			return B_NO_MEMORY;
	}

	return B_OK;
}

// _RestoreStyleable
void
MessageImporter::_RestoreStyleable(Styleable* styleable,
	const BMessage& archive) const
{
	BMessage styleArchive;
	if (archive.FindMessage("style", &styleArchive) != B_OK)
		return;
	
	BaseObjectRef styleRef = ImportObject(styleArchive);
	Style* style = dynamic_cast<Style*>(styleRef.Get());
	if (style != NULL)
		styleable->SetStyle(style);
}

