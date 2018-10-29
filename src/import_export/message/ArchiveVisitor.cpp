/*
 * Copyright 2013-2014, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "ArchiveVisitor.h"

#include <ByteOrder.h>
#include <DataIO.h>
#include <Message.h>
#include <TypeConstants.h>

#include "bitmap_compression.h"
#include "Brush.h"
#include "CharacterStyle.h"
#include "Color.h"
#include "ColorShade.h"
#include "Font.h"
#include "Gradient.h"
#include "StyleRun.h"
#include "StyleRunList.h"

static const char* kType = "type";

ArchiveVisitor::ArchiveVisitor(const DocumentRef& document, BMessage* archive)
	: fDocument(document)
	, status(B_OK)
{
	VisitDocument(fDocument.Get(), archive);
}	

bool
ArchiveVisitor::VisitDocument(Document* document, BMessage* context)
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

bool
ArchiveVisitor::VisitLayer(Layer* layer, BMessage* context)
{
	BMessage archive;
	if (!inherited::VisitLayer(layer, &archive))
		return false;

	status = archive.AddString(kType, "Layer");

	if (status == B_OK)
		status = _StoreObject(layer, &archive);

	if (status == B_OK)
		status = context->AddMessage("object", &archive);

	return status == B_OK;
}

bool
ArchiveVisitor::VisitObject(Object* object, BMessage* context)
{
	BMessage archive;
	if (!inherited::VisitObject(object, &archive))
		return false;
	
	status = _StoreObject(object, &archive);

	if (status == B_OK)
		status = context->AddMessage("object", &archive);
	
	return status == B_OK;
}

bool
ArchiveVisitor::VisitBoundedObject(BoundedObject* boundedObject,
	BMessage* context)
{
	if (boundedObject->Opacity() < 255) {
		status = context->AddUInt8("opacity", boundedObject->Opacity());
		if (status != B_OK)
			return false;
	}
	return inherited::VisitBoundedObject(boundedObject, context);
}

bool
ArchiveVisitor::VisitFilter(Filter* filter, BMessage* context)
{
	status = context->AddString(kType, "FilterGaussianBlur");
	if (status == B_OK)
		status = context->AddFloat("radius", filter->FilterRadius());
	return status == B_OK;
}

bool
ArchiveVisitor::VisitFilterBrightness(FilterBrightness* brightness,
	BMessage* context)
{
	status = context->AddString(kType, "FilterBrightness");
	if (status == B_OK)
		status = context->AddInt32("offset", brightness->Offset());
	if (status == B_OK)
		status = context->AddFloat("factor", brightness->Factor());
	return status == B_OK;
}

bool
ArchiveVisitor::VisitFilterDropShadow(FilterDropShadow* dropShadow,
	BMessage* context)
{
	status = context->AddString(kType, "FilterDropShadow");
	if (status == B_OK)
		status = context->AddFloat("radius", dropShadow->FilterRadius());
	if (status == B_OK)
		status = context->AddFloat("offset-x", dropShadow->OffsetX());
	if (status == B_OK)
		status = context->AddFloat("offset-y", dropShadow->OffsetY());
	if (status == B_OK)
		status = context->AddFloat("opacity", dropShadow->Opacity());
	if (status == B_OK && dropShadow->Color().Get() != NULL) {
		BMessage colorArchive;
		status = _StoreColorProvider(dropShadow->Color().Get(),
			&colorArchive);
		if (status == B_OK)
			status = context->AddMessage("color", &colorArchive);
	}
	return status == B_OK;
}

bool
ArchiveVisitor::VisitFilterSaturation(FilterSaturation* saturation,
	BMessage* context)
{
	status = context->AddString(kType, "FilterSaturation");
	if (status == B_OK)
		status = context->AddFloat("saturation", saturation->Saturation());
	return status == B_OK;
}

bool
ArchiveVisitor::VisitBrushStroke(BrushStroke* brushStroke, BMessage* context)
{
	status = context->AddString(kType, "BrushStroke");
	if (status == B_OK) {
		BMessage brushArchive;
		status = _StoreResourceOrIndex(brushStroke->Brush(), &brushArchive);
		if (status == B_OK)
			status = context->AddMessage("brush", &brushArchive);
	}

	if (status == B_OK) {
		BMessage paintArchive;
		status = _StoreResourceOrIndex(brushStroke->Paint(), &paintArchive);
		if (status == B_OK)
			status = context->AddMessage("paint", &paintArchive);
	}	

	if (status == B_OK) {
		BMessage strokeArchive;
		const Stroke& stroke = brushStroke->Stroke();
		int32 count = stroke.CountObjects();
		for (int32 i = 0; i < count; i++) {
			StrokePoint* point = stroke.ObjectAt(i);
			status = strokeArchive.AddPoint("point", point->point);
			if (status == B_OK) {
				status = strokeArchive.AddFloat("pressure",
					point->pressure);
			}
			if (status == B_OK)
				status = strokeArchive.AddFloat("tilt-x", point->tiltX);
			if (status == B_OK)
				status = strokeArchive.AddFloat("tilt-y", point->tiltY);
			if (status != B_OK)
				break;
		}

		if (status == B_OK)
			status = context->AddMessage("stroke", &strokeArchive);
	}	

	return status == B_OK;
}

bool
ArchiveVisitor::VisitImage(Image* image, BMessage* context)
{
	status = context->AddString(kType, "Image");
	if (status == B_OK)
		status = archive_buffer(image->Buffer(), context, "bitmap");
	return status == B_OK;
}

bool
ArchiveVisitor::VisitStyleable(Styleable* styleable, BMessage* context)
{
	BMessage styleArchive;
	status = _StoreResourceOrIndex(styleable->Style(), &styleArchive);
	if (status == B_OK)
		status = context->AddMessage("style", &styleArchive);
	if (status != B_OK)
		return false;
	return inherited::VisitStyleable(styleable, context);
}

bool
ArchiveVisitor::VisitRect(Rect* rect, BMessage* context)
{
	status = context->AddString(kType, "Rect");
	if (status == B_OK)
		status = context->AddRect("area", rect->Area());
	if (status == B_OK && rect->RoundCornerRadius() != 0.0)
		status = context->AddDouble("radius", rect->RoundCornerRadius());
	return status == B_OK;
}

bool
ArchiveVisitor::VisitShape(Shape* shape, BMessage* context)
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

bool
ArchiveVisitor::VisitText(Text* text, BMessage* context)
{
	status = context->AddString(kType, "Text");
	if (status == B_OK && text->GetCharCount() > 0)
		status = context->AddString("text", text->GetText());
	if (status == B_OK && text->Width() != 0.0)
		status = context->AddDouble("width", text->Width());
	if (status == B_OK && text->Alignment() != 0)
		status = context->AddUInt32("alignment", text->Alignment());
	if (status == B_OK && text->GlyphSpacing() != 0.0)
		status = context->AddDouble("glyph spacing", text->GlyphSpacing());

	if (status == B_OK)
		status = _StoreStyleRuns(text, context);

	return status == B_OK;
}

// #pragma mark -

status_t
ArchiveVisitor::_StoreObject(Object* object, BMessage* archive) const
{
	status_t ret = B_OK;

	const BString& name = object->GivenName();
	if (name.Length() > 0)
		ret = archive->AddString("name", name);

	if (!object->IsVisible())
		ret = archive->AddBool("visible", false);

	if (ret == B_OK && !object->IsIdentity()) {
		double matrix[Transformable::MatrixSize];
		object->StoreTo(matrix);
		ret = archive->AddData("matrix", B_DOUBLE_TYPE,
			matrix, sizeof(matrix));
	}

	return ret;
}

status_t
ArchiveVisitor::_StoreResources(const ResourceList& resources,
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

status_t
ArchiveVisitor::_StoreResource(BaseObject* object, BMessage* archive) const
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

	Paint* paint = dynamic_cast<Paint*>(object);
	if (paint != NULL)
		return _StorePaint(paint, archive);

	fprintf(stderr, "Unkown resource object type!\n");
	return B_OK;
}

status_t
ArchiveVisitor::_StorePath(Path* path, BMessage* archive) const
{
	status_t ret = path->Archive(archive, true);
	if (ret != B_OK)
		return ret;
	return archive->AddString(kType, "Path");
}

status_t
ArchiveVisitor::_StoreStyle(Style* style, BMessage* archive) const
{
	status_t ret = archive->AddString(kType, "Style");

	Paint* fillPaint = style->FillPaint();
	if (ret == B_OK && fillPaint != NULL
		&& fillPaint->Type() != Paint::NONE) {
		BMessage paintArchive;
		ret = _StoreResourceOrIndex(fillPaint, &paintArchive);
		if (ret == B_OK)
			ret = archive->AddMessage("fill paint", &paintArchive);
	}

	Paint* strokePaint = style->StrokePaint();
	if (ret == B_OK && strokePaint != NULL
		&& strokePaint->Type() != Paint::NONE) {
		BMessage paintArchive;
		ret = _StoreResourceOrIndex(strokePaint, &paintArchive);
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

status_t
ArchiveVisitor::_StorePaint(Paint* paint, BMessage* archive) const
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
				ret = _StoreResourceOrIndex(ref.Get(), &providerArchive);
				if (ret == B_OK)
					ret = archive->AddMessage("color", &providerArchive);
			}
			break;
		}

		case Paint::GRADIENT:
		{
			const GradientRef& gradient = paint->Gradient();
			if (gradient.Get() != NULL) {
				BMessage gradientArchive;
				ret = _StoreGradient(gradient.Get(), &gradientArchive);
				if (ret == B_OK)
					ret = archive->AddMessage("gradient", &gradientArchive);
			}
			break;
		}

		case Paint::PATTERN:
			fprintf(stderr,
				"MessageExporter::_StorePaint() - Implement PATTERN!\n");
			break;

		case Paint::ERASE:
			ret = archive->AddBool("erase", true);
			break;
	}

	if (ret == B_OK)
		ret = archive->AddString(kType, "Paint");

	return ret;
}

status_t
ArchiveVisitor::_StoreStrokeProperties(StrokeProperties* properties,
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

status_t
ArchiveVisitor::_StoreColorProvider(ColorProvider* provider,
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

status_t
ArchiveVisitor::_StoreColor(Color* color, BMessage* archive) const
{
	const rgb_color& rgba = color->GetColor();
	
	status_t ret = B_OK;
	if (ret == B_OK)
		ret = archive->AddUInt8("r", rgba.red);
	if (ret == B_OK)
		ret = archive->AddUInt8("g", rgba.green);
	if (ret == B_OK)
		ret = archive->AddUInt8("b", rgba.blue);
	if (ret == B_OK)
		ret = archive->AddUInt8("a", rgba.alpha);

	if (ret == B_OK)
		ret = archive->AddString(kType, "Color");
		
	return ret;
}

status_t
ArchiveVisitor::_StoreColorShade(ColorShade* shade, BMessage* archive) const
{
	status_t ret = B_OK;

	if (ret == B_OK)
		ret = archive->AddFloat("h", shade->Hue());
	if (ret == B_OK)
		ret = archive->AddFloat("s", shade->Saturation());
	if (ret == B_OK)
		ret = archive->AddFloat("v", shade->Value());

	const ColorProviderRef& provider = shade->GetColorProvider();
	if (ret == B_OK) {
		BMessage providerArchive;
		ret = _StoreResourceOrIndex(provider.Get(), &providerArchive);
		if (ret == B_OK)
			ret = archive->AddMessage("provider", &providerArchive);
	}

	if (ret == B_OK)
		ret = archive->AddString(kType, "ColorShade");
		
	return ret;
}

status_t
ArchiveVisitor::_StoreGradient(const Gradient* gradient,
	BMessage* archive) const
{
	status_t ret = gradient->Archive(archive, true);
	if (ret != B_OK)
		return ret;
	return archive->AddString(kType, "Gradient");
}

status_t
ArchiveVisitor::_StoreBrush(Brush* brush, BMessage* archive) const
{
	status_t ret = brush->Archive(archive, true);
	if (ret != B_OK)
		return ret;
	return archive->AddString(kType, "Brush");
}

status_t
ArchiveVisitor::_StoreStyleRuns(Text* text, BMessage* archive) const
{
	const StyleRunList& styleRuns = text->GetStyleRuns();
	int32 runCount = styleRuns.CountRuns();
	for (int32 i = 0; i < runCount; i++) {
		const StyleRun* run = styleRuns.StyleRunAt(i);
		BMessage runArchive;
		status_t ret = _StoreStyleRun(run, &runArchive);
		
		if (ret == B_OK)
			ret = archive->AddMessage("style run", &runArchive);

		if (ret != B_OK)
			return ret;
	}
	return B_OK;
}

status_t
ArchiveVisitor::_StoreStyleRun(const StyleRun* run, BMessage* archive) const
{
	const CharacterStyleRef& style = run->GetStyle();
	if (style.Get() == NULL)
		return B_BAD_VALUE;

	const Font& font = style->GetFont();
	
	status_t ret = archive->AddInt32("length", run->GetLength());

	if (ret == B_OK)
		ret = archive->AddString("font family", font.getFamily());
	if (ret == B_OK)
		ret = archive->AddString("font style", font.getStyle());
	if (ret == B_OK)
		ret = archive->AddDouble("font size", font.getSize());
	if (ret == B_OK) {
		ret = archive->AddInt32("font script level",
			(int32)font.getScriptLevel());
	}

	if (ret == B_OK && style->GetGlyphSpacing() != 0.0)
		ret = archive->AddDouble("glyph spacing", style->GetGlyphSpacing());
	if (ret == B_OK && style->GetFauxWeight() != 0.0)
		ret = archive->AddDouble("faux weight", style->GetFauxWeight());
	if (ret == B_OK && style->GetFauxItalic() != 0.0)
		ret = archive->AddDouble("faux italic", style->GetFauxItalic());

	if (ret == B_OK && style->GetStyle().Get() != NULL) {
		BMessage styleArchive;
		ret = _StoreStyle(style->GetStyle().Get(), &styleArchive);
		if (ret == B_OK)
			ret = archive->AddMessage("style", &styleArchive);
	}

	return ret;
}

status_t
ArchiveVisitor::_StoreResourceOrIndex(BaseObject* object,
	BMessage* archive) const
{
	if (object == NULL)
		return B_OK;
	
	int32 index = _GlobalResourceIndex(object);
	if (index >= 0)
		return archive->AddInt32("resource", index);

	return _StoreResource(object, archive);
}

int32
ArchiveVisitor::_GlobalResourceIndex(BaseObject* object) const
{
	return fDocument->GlobalResources().IndexOf(object);
}
