/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "MessageImporter.h"

#include <ByteOrder.h>
#include <DataIO.h>
#include <Message.h>
#include <TypeConstants.h>

#include "DocumentVisitor.h"
#include "Color.h"
#include "ColorShade.h"

// constructor
MessageImporter::MessageImporter(const DocumentRef& document)
	: fDocument(document)
{
}

// destructor
MessageImporter::~MessageImporter()
{
}

static const char* kType = "type";

class ArchiveVisitor : public DocumentVisitor<BMessage> {
	typedef DocumentVisitor<BMessage> inherited;
	
public:
	ArchiveVisitor(const DocumentRef& document, BMessage* archive)
		: fDocument(document)
		, status(B_OK)
	{
		VisitDocument(fDocument.Get(), archive);
	}	
	
	virtual bool VisitDocument(Document* document, BMessage* context)
	{
		status = context->AddRect("bounds", document->Bounds());
		if (status == B_OK) {
			BMessage resources;
			status = _StoreResources(document->GlobalResources(), &resources);
			if (status == B_OK)
				status = context->AddMessage("resources", &resources);
		}
		
		return status == B_OK && !inherited::VisitDocument(document, context);
	}

	virtual bool VisitLayer(Layer* layer, BMessage* context)
	{
		BMessage archive;
		if (!inherited::VisitLayer(layer, &archive))
			return false;

		status = archive.AddString(kType, "Layer");
		if (status == B_OK)
			status = context->AddMessage("object", &archive);

		return status == B_OK;
	}

	virtual bool VisitObject(Object* object, BMessage* context)
	{
		BMessage archive;
		if (!inherited::VisitObject(object, &archive))
			return false;
		
		status = B_OK;

		const BString& name = object->GivenName();
		if (status == B_OK && name.Length() > 0)
			status = archive.AddString("name", name);

		if (status == B_OK)
			status = context->AddMessage("object", &archive);
		
		return status == B_OK;
	}

	virtual bool VisitBoundedObject(BoundedObject* boundedObject,
		BMessage* context)
	{
		return inherited::VisitBoundedObject(boundedObject, context);
	}

	virtual bool VisitFilter(Filter* filter, BMessage* context)
	{
		status = context->AddString(kType, "FilterGaussianBlur");
		return status == B_OK;
	}

	virtual bool VisitFilterDropShadow(FilterDropShadow* dropShadow,
		BMessage* context)
	{
		status = context->AddString(kType, "FilterDropShadow");
		return status == B_OK;
	}

	virtual bool VisitFilterSaturation(FilterSaturation* saturation,
		BMessage* context)
	{
		status = context->AddString(kType, "FilterSaturation");
		return status == B_OK;
	}

	virtual bool VisitBrushStroke(BrushStroke* stroke, BMessage* context)
	{
		status = context->AddString(kType, "BrushStroke");
		return status == B_OK;
	}

	virtual bool VisitImage(Image* image, BMessage* context)
	{
		status = context->AddString(kType, "Image");
		return status == B_OK;
	}

	virtual bool VisitStyleable(Styleable* styleable, BMessage* context)
	{
		BMessage styleArchive;
		status = _StoreResourceOrIndex(styleable->Style(), &styleArchive);
		if (status == B_OK)
			status = context->AddMessage("style", &styleArchive);
		if (status != B_OK)
			return false;
		return inherited::VisitStyleable(styleable, context);
	}

	virtual bool VisitRect(Rect* rect, BMessage* context)
	{
		status = context->AddString(kType, "Rect");
		if (status == B_OK)
			status = context->AddRect("area", rect->Area());
		if (status == B_OK && rect->RoundCornerRadius() != 0.0)
			status = context->AddDouble("radius", rect->RoundCornerRadius());
		return status == B_OK;
	}

	virtual bool VisitShape(Shape* shape, BMessage* context)
	{
		status = context->AddString(kType, "Shape");
		const PathList& paths = shape->Paths();
		int32 pathCount = paths.CountItems();
		for (int32 i = 0; i < pathCount; i++) {
			BMessage pathArchive;
			const PathRef& path = paths.ItemAtFast(i);
			printf("path: %p\n", path.Get());
			status = _StoreResourceOrIndex(path.Get(), &pathArchive);
			if (status != B_OK)
				break;
			status = context->AddMessage("path", &pathArchive);
		}
		return status == B_OK;
	}

	virtual bool VisitText(Text* text, BMessage* context)
	{
		status = context->AddString(kType, "Text");
		if (status == B_OK && text->GetCharCount() > 0)
			status = context->AddString("text", text->GetText());
		return status == B_OK;
	}

private:
	status_t _StoreResources(const ResourceList& resources,
		BMessage* archive) const
	{
		int32 count = resources.CountObjects();
		for (int32 i = 0; i < count; i++) {
			BaseObject* object = resources.ObjectAtFast(i);
			printf("storing: %p\n", object);
			BMessage objectArchive;
			status_t ret = _StoreResource(object, &objectArchive);
			if (ret == B_OK)
				ret = archive->AddMessage("object", &objectArchive);
			if (ret != B_OK)
				return ret;
		}
		return B_OK;
	}

	status_t _StoreResource(BaseObject* object, BMessage* archive) const
	{
		Path* path = dynamic_cast<Path*>(object);
		if (path != NULL)
			return _StorePath(path, archive);

		Style* style = dynamic_cast<Style*>(object);
		if (style != NULL)
			return _StoreStyle(style, archive);

		ColorProvider* provider = dynamic_cast<ColorProvider*>(object);
		if (provider != NULL)
			return _StoreColorProvider(provider, archive);

		Brush* brush = dynamic_cast<Brush*>(object);
		if (brush != NULL)
			return _StoreBrush(brush, archive);
	
		fprintf(stderr, "Unkown resource object type!\n");
		return B_OK;
	}

	status_t _StorePath(Path* path, BMessage* archive) const
	{
		status_t ret = path->Archive(archive, true);
		if (ret != B_OK)
			return ret;
		return archive->AddString(kType, "Path");
	}

	status_t _StoreStyle(Style* style, BMessage* archive) const
	{
		status_t ret = archive->AddString(kType, "Style");

		Paint* fillPaint = style->FillPaint();
		if (ret == B_OK && fillPaint != NULL
			&& fillPaint->Type() != Paint::NONE) {
			BMessage paintArchive;
			ret = _StorePaint(fillPaint, &paintArchive);
			if (ret == B_OK)
				ret = archive->AddMessage("fill paint", &paintArchive);
		}

		Paint* strokePaint = style->StrokePaint();
		if (ret == B_OK && strokePaint != NULL
			&& strokePaint->Type() != Paint::NONE) {
			BMessage paintArchive;
			ret = _StorePaint(strokePaint, &paintArchive);
			if (ret == B_OK)
				ret = archive->AddMessage("stroke paint", &paintArchive);
		}

		StrokeProperties* strokeProperties = style->StrokeProperties();
		if (ret == B_OK && strokeProperties != NULL
			&& strokeProperties->SetProperties() != 0) {
			BMessage propertiesArchive;
			ret = _StoreStrokeProperties(strokeProperties, &propertiesArchive);
			if (ret == B_OK) {
				ret = archive->AddMessage("stroke properties",
					&propertiesArchive);
			}
		}

		return ret;
	}

	status_t _StorePaint(Paint* paint, BMessage* archive) const
	{
		status_t ret = B_OK;

		switch (paint->Type()) {
			default:
			case Paint::NONE:
				break;

			case Paint::COLOR:
			{
				const ColorProviderRef& ref = paint->GetColorProvider();
				if (ref.Get() != NULL) {
					BMessage providerArchive;
					ret = _StoreColorProvider(ref.Get(), &providerArchive);
					if (ret == B_OK)
						ret = archive->AddMessage("color", &providerArchive);
				}
				break;
			}

			case Paint::GRADIENT:
				fprintf(stderr,
					"MessageImporter::_StorePaint() - Implement GRADIENT!\n");
				break;

			case Paint::PATTERN:
				fprintf(stderr,
					"MessageImporter::_StorePaint() - Implement PATTERN!\n");
				break;
		}

		return ret;
	}

	status_t _StoreStrokeProperties(StrokeProperties* properties,
		BMessage* archive) const
	{
		uint32 flags = properties->SetProperties();
		status_t ret = B_OK;

		if (ret == B_OK && (flags & STROKE_WIDTH) != 0)
			ret = archive->AddFloat("width", properties->Width());

		if (ret == B_OK && (flags & STROKE_MITER_LIMIT) != 0)
			ret = archive->AddFloat("miter limit", properties->MiterLimit());

		if (ret == B_OK && (flags & STROKE_CAP_MODE) != 0)
			ret = archive->AddInt32("cap mode", properties->CapMode());

		if (ret == B_OK && (flags & STROKE_JOIN_MODE) != 0)
			ret = archive->AddInt32("join mode", properties->JoinMode());

		if (ret == B_OK && (flags & STROKE_POSITION) != 0)
			ret = archive->AddInt32("position", properties->StrokePosition());

		if (ret == B_OK)
			ret = archive->AddString(kType, "StrokeProperties");

		return ret;
	}

	status_t _StoreColorProvider(ColorProvider* provider,
		BMessage* archive) const
	{
		Color* color = dynamic_cast<Color*>(provider);
		if (color != NULL)
			return _StoreColor(color, archive);

		ColorShade* shade = dynamic_cast<ColorShade*>(provider);
		if (shade != NULL)
			return _StoreColorShade(shade, archive);
	
		fprintf(stderr, "Unkown ColorProvider type!\n");
		return B_OK;
	}

	status_t _StoreColor(Color* color, BMessage* archive) const
	{
		const rgb_color& rgba = color->GetColor();
		
		status_t ret = B_OK;
		if (ret == B_OK)
			ret = archive->AddInt32("r", rgba.red);
		if (ret == B_OK)
			ret = archive->AddInt32("g", rgba.green);
		if (ret == B_OK)
			ret = archive->AddInt32("b", rgba.blue);
		if (ret == B_OK)
			ret = archive->AddInt32("a", rgba.alpha);

		if (ret == B_OK)
			ret = archive->AddString(kType, "Color");
			
		return ret;
	}

	status_t _StoreColorShade(ColorShade* shade, BMessage* archive) const
	{
		status_t ret = B_OK;

		if (ret == B_OK)
			ret = archive->AddFloat("h", shade->Hue());
		if (ret == B_OK)
			ret = archive->AddFloat("s", shade->Saturation());
		if (ret == B_OK)
			ret = archive->AddFloat("v", shade->Value());

		const ColorProviderRef& provider = shade->GetColorProvider();
		if (ret == B_OK)
			ret = _StoreResourceOrIndex(provider.Get(), archive);

		if (ret == B_OK)
			ret = archive->AddString(kType, "ColorShade");
			
		return ret;
	}

	status_t _StoreBrush(Brush* brush, BMessage* archive) const
	{
		status_t ret = brush->Archive(archive, true);
		if (ret != B_OK)
			return ret;
		return archive->AddString(kType, "Brush");
	}

	status_t _StoreResourceOrIndex(BaseObject* object, BMessage* archive) const
	{
		if (object == NULL)
			return B_OK;
		
		int32 index = _GlobalResourceIndex(object);
		if (index >= 0)
			return archive->AddInt32("resource", index);
	
		return _StoreResource(object, archive);
	}

	int32 _GlobalResourceIndex(BaseObject* object) const
	{
		return fDocument->GlobalResources().IndexOf(object);
	}

private:
	DocumentRef		fDocument;

public:
	status_t		status;
};

// Import
status_t
MessageImporter::Import(BPositionIO* stream) const
{
	if (fDocument.Get() == NULL)
		return B_NO_INIT;

	uint32 magic = 0;
	ssize_t size = sizeof(magic);
	ssize_t read = stream->Read(&magic, size);
	if (read != size) {
		if (read < 0)
			return (status_t)read;
		else
			return B_IO_ERROR;
	}

	if (B_BENDIAN_TO_HOST_INT32(magic) != 'WBI2')
		return B_BAD_VALUE;

	BMessage archive;
	status_t ret = archive.Unflatten(stream);
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

	return ImportObjects(archive);
}

// ImportGlobalResources
status_t
MessageImporter::ImportGlobalResources(const BMessage& archive) const
{
	for (int32 i = 0;; i++) {
		BMessage objectArchive;
		status_t ret = archive.FindMessage("object", i, &objectArchive);
		if (ret != B_OK)
			break;

		BaseObjectRef object = ImportObject(objectArchive);
		if (object.Get() == NULL)
			continue;

		if (!fDocument->GlobalResources().AddObject(object.Get()))
			return B_NO_MEMORY;
	}

	return B_OK;
}

// ImportObjects
status_t
MessageImporter::ImportObjects(const BMessage& archive) const
{
	return B_OK;
}

// ImportObject
BaseObjectRef
MessageImporter::ImportObject(const BMessage& archive) const
{
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
	return BaseObjectRef();
}

// ImportRect
BaseObjectRef
MessageImporter::ImportRect(const BMessage& archive) const
{
	return BaseObjectRef();
}

// ImportShape
BaseObjectRef
MessageImporter::ImportShape(const BMessage& archive) const
{
	return BaseObjectRef();
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

