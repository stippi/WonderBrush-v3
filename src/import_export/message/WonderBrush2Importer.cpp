/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "WonderBrush2Importer.h"

#include <Bitmap.h>
#include <DataIO.h>
#include <Message.h>
#include <TypeConstants.h>

#include "bitmap_compression.h"
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
#include "Font.h"
#include "Gradient.h"
#include "Image.h"
#include "Layer.h"
#include "Object.h"
#include "Paint.h"
#include "RenderBuffer.h"
#include "Shape.h"
#include "StrokeProperties.h"
#include "Styleable.h"
#include "StyleRun.h"
#include "StyleRunList.h"
#include "Rect.h"
#include "Text.h"
#include "ui_defines.h"

// constructor
WonderBrush2Importer::WonderBrush2Importer(const DocumentRef& document)
	: fDocument(document)
{
}

// destructor
WonderBrush2Importer::~WonderBrush2Importer()
{
}

// Import
status_t
WonderBrush2Importer::Import(BPositionIO& stream) const
{
	if (fDocument.Get() == NULL)
		return B_NO_INIT;

	BMessage archive;
	status_t ret = archive.Unflatten(&stream);
	if (ret != B_OK)
		return ret;

	archive.PrintToStream();

	return ImportDocument(archive);
}

// ImportDocument
status_t
WonderBrush2Importer::ImportDocument(const BMessage& archive) const
{
	// Import document settings
	BRect bounds;
	if (archive.FindRect("bounds", &bounds) == B_OK)
		fDocument->SetBounds(bounds);

	// int32 "current layer"
	// float "zoom level"
	// BPoint "zoom center"
	// bool "show guides"

	// Import layers
	for (int32 i = 0;; i++) {
		BMessage layerArchive;
		if (archive.FindMessage("layer", i, &layerArchive) != B_OK)
			break;
		BaseObjectRef ref = ImportLayer(layerArchive);
		Layer* layer = dynamic_cast<Layer*>(ref.Get());
		// Add layers in reverse order
		if (layer == NULL || !fDocument->RootLayer()->AddObject(layer, 0))
			return B_NO_MEMORY;
	}

	return B_OK;
}

// ImportObjects
status_t
WonderBrush2Importer::ImportObjects(const BMessage& archive, Layer* layer) const
{
	return _ImportObjects<Object, Layer>(archive, layer);
}

// ImportObject
BaseObjectRef
WonderBrush2Importer::ImportObject(const BMessage& archive) const
{
	BString type;
	if (archive.FindString("class", &type) != B_OK) {
		fprintf(stderr, "WonderBrush2Importer::ImportObject() - "
			"Type string not found!\n");
		return BaseObjectRef();
	}

	// objects
	if (type == "BrushStroke")
		return ImportBrushStroke(archive);

	if (type == "GaussianBlur")
		return ImportFilterGaussianBlur(archive);
		
	if (type == "DropShadow")
		return ImportFilterDropShadow(archive);

	if (type == "FilterSaturation")
		return ImportFilterSaturation(archive);

	if (type == "BitmapStroke")
		return ImportImage(archive);

	if (type == "ShapeStroke")
		return ImportShape(archive);

	if (type == "TextStroke")
		return ImportText(archive);

	// fills
	if (type == "Brush")
		return ImportBrush(archive);

	if (type == "VectorPath")
		return ImportPath(archive);

	if (type == "StrokeProperties")
		return ImportStrokeProperties(archive);

	if (type == "ColorRenderer")
		return ImportColorRenderer(archive);

	if (type == "GradientRenderer")
		return ImportGradientRenderer(archive);

	if (type == "EraseRenderer")
		return ImportEraseRenderer(archive);

	// unhandled!
	fprintf(stderr, "WonderBrush2Importer::ImportObject() - "
		"Unkown object type: '%s'\n", type.String());

	return BaseObjectRef();
}

// ImportBrushStroke
BaseObjectRef
WonderBrush2Importer::ImportBrushStroke(const BMessage& archive) const
{
	BrushStroke* brushStroke = new BrushStroke();
	if (brushStroke != NULL) {
		Brush* brush = new(std::nothrow) Brush();
		if (brush != NULL) {
			brushStroke->SetBrush(brush);
			float value;
			if (archive.FindFloat("min alpha", &value) == B_OK)
				brush->SetMinOpacity(value);
			if (archive.FindFloat("max alpha", &value) == B_OK)
				brush->SetMaxOpacity(value);
//			if (archive.FindFloat("min spacing", &value) == B_OK)
//				brush->Set(value);
//			if (archive.FindFloat("max spacing", &value) == B_OK)
//				brush->Set(value);
			if (archive.FindFloat("min radius", &value) == B_OK)
				brush->SetMinRadius(value);
			if (archive.FindFloat("max radius", &value) == B_OK)
				brush->SetMaxRadius(value);
			if (archive.FindFloat("min hardness", &value) == B_OK)
				brush->SetMinHardness(value);
			if (archive.FindFloat("max hardness", &value) == B_OK)
				brush->SetMaxHardness(value);
			
			brush->RemoveReference();
		}

		BMessage rendererArchive;
		if (archive.FindMessage("renderer", &rendererArchive) == B_OK) {
			BaseObjectRef ref = ImportObject(rendererArchive);
			Paint* paint = dynamic_cast<Paint*>(ref.Get());
			if (paint != NULL)
				brushStroke->SetPaint(paint);
		}

		const void* data;
		ssize_t size;
		for (int32 i = 0;; i++) {
			if (archive.FindData("points", B_RAW_TYPE, i, &data, &size) == B_OK
				&& size == sizeof(StrokePoint)) {
				const StrokePoint* strokePoint = (const StrokePoint*)data;
				if (!brushStroke->AppendPoint(*strokePoint))
					break;
			} else
				break;
		}

		_RestoreBoundedObject(brushStroke, archive);
	}
	return BaseObjectRef(brushStroke, true);
}

// ImportFilterGaussianBlur
BaseObjectRef
WonderBrush2Importer::ImportFilterGaussianBlur(const BMessage& archive) const
{
	Filter* filter = new(std::nothrow) Filter();
	if (filter != NULL) {
		float radius;
		if (archive.FindFloat("blur radius", &radius) == B_OK)
			filter->SetFilterRadius(radius);
		
		_RestoreObject(filter, archive);
	}
	return BaseObjectRef(filter, true);
}

// ImportFilterDropShadow
BaseObjectRef
WonderBrush2Importer::ImportFilterDropShadow(const BMessage& archive) const
{
	FilterDropShadow* filter = new(std::nothrow) FilterDropShadow();
	if (filter != NULL) {
		float radius;
		if (archive.FindFloat("radius", &radius) == B_OK)
			filter->SetFilterRadius(radius);
		
		BPoint offset;
		if (archive.FindPoint("offset", &offset) == B_OK) {
			filter->SetOffsetX(offset.x);
			filter->SetOffsetY(offset.y);
		}
		int32 opacity;
		if (archive.FindInt32("opacity", &opacity) == B_OK)
			filter->SetOpacity(opacity);

		const void* data;
		ssize_t size;
		if (archive.FindData("RGBColor", B_RGB_COLOR_TYPE, &data,
				&size) == B_OK && size == sizeof(rgb_color)) {
			rgb_color rgba = *(const rgb_color*)data;
			Color* color = new(std::nothrow) Color(rgba);
			if (color != NULL)
				filter->SetColor(ColorProviderRef(color, true));
		}

		_RestoreObject(filter, archive);
	}
	return BaseObjectRef(filter, true);
}

// ImportFilterSaturation
BaseObjectRef
WonderBrush2Importer::ImportFilterSaturation(const BMessage& archive) const
{
	FilterSaturation* filter = new(std::nothrow) FilterSaturation();
	if (filter != NULL) {
		float saturation;
		if (archive.FindFloat("saturation", &saturation) == B_OK)
			filter->SetSaturation(saturation);
		
		_RestoreObject(filter, archive);
	}
	return BaseObjectRef(filter, true);
}

// ImportImage
BaseObjectRef
WonderBrush2Importer::ImportImage(const BMessage& archive) const
{
	Image* image = new(std::nothrow) Image();
	if (image != NULL) {
		BBitmap* bitmap;
		if (extract_bitmap(&bitmap, &archive, "bitmap") == B_OK) {
			image->SetBuffer(RenderBufferRef(
				new(std::nothrow) RenderBuffer(bitmap), true));
			delete bitmap;
		} else {
			fprintf(stderr, "WonderBrush2Importer::ImportImage() - "
				"failed to extract bitmap buffer!\n");
		}

		_RestoreBoundedObject(image, archive);
	}
	return BaseObjectRef(image, true);
}

// ImportLayer
BaseObjectRef
WonderBrush2Importer::ImportLayer(const BMessage& archive) const
{
	Layer* layer = new(std::nothrow) Layer(fDocument->Bounds());
	if (layer != NULL) {
		ImportObjects(archive, layer);

		// TODO: int32 "mode" (blending mode)
		int32 flags;
		if (archive.FindInt32("flags", &flags) == B_OK) {
			if ((flags & 0x01) != 0)
				layer->SetVisible(false);
		}

		_RestoreObject(layer, archive);
	}
	return BaseObjectRef(layer, true);
}

// ImportShape
BaseObjectRef
WonderBrush2Importer::ImportShape(const BMessage& archive) const
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

		Style* style = shape->Style();

		bool outline;
		if (archive.FindBool("outline", &outline) != B_OK)
			outline = false;
		
		if (outline) {
			style->FillPaint()->SetType(Paint::NONE);
			
			StrokeProperties* strokeProperties = style->StrokeProperties();
			int32 capMode;
			if (archive.FindInt32("cap mode", &capMode) == B_OK)
				strokeProperties->SetCapMode((CapMode)capMode);
			int32 joinMode;
			if (archive.FindInt32("join mode", &joinMode) == B_OK)
				strokeProperties->SetJoinMode((JoinMode)joinMode);
			float width;
			if (archive.FindFloat("outline width", &width) == B_OK)
				strokeProperties->SetWidth(width);
		} else {
			style->StrokePaint()->SetType(Paint::NONE);
		}
	}
	return BaseObjectRef(shape, true);
}

// ImportText
BaseObjectRef
WonderBrush2Importer::ImportText(const BMessage& archive) const
{
	Text* text = new(std::nothrow) Text(kBlack);
	if (text != NULL) {
		BMessage textArchive;
		status_t ret = archive.FindMessage("text renderer", &textArchive);
		
		BString string;
		if (ret == B_OK)
			ret = textArchive.FindString("text", &string);

		if (ret == B_OK) {
			float width;
			if (textArchive.FindFloat("text width", &width) == B_OK)
				text->SetWidth(width);

			int32 alignment;
			if (textArchive.FindInt32("text alignment", &alignment) == B_OK)
				text->SetAlignment(alignment);

			float glyphSpacing;
			if (textArchive.FindFloat("advance scale", &glyphSpacing) == B_OK)
				text->SetGlyphSpacing(glyphSpacing);
		}

		const char* fontFamily;
		if (ret == B_OK)
			ret = textArchive.FindString("family", &fontFamily);

		const char* fontStyle;
		if (ret == B_OK)
			ret = textArchive.FindString("style", &fontStyle);

		float fontSize;
		if (ret == B_OK)
			ret = textArchive.FindFloat("size", &fontSize);

		BMessage styleArchive;
		if (ret == B_OK)
			ret = archive.FindMessage("renderer", &styleArchive);

		if (ret == B_OK) {
			StyleRef style = _ImportStyle(styleArchive);
			if (ret == B_OK && style.Get() != NULL) {
				Font font(fontFamily, fontStyle, fontSize * 16.0f);
				text->SetText(string, font, style);
			}
		}
	
		_RestoreStyleable(text, archive);
		
		if (ret == B_OK) {
			// Offset by ascent. WonderBrush 2 had the origin at the
			// base of the first line. TODO: By *ascent* not size!
			text->TranslateBy(BPoint(0.0, -fontSize * 16.0f));
		}
	}
	return BaseObjectRef(text, true);
}

// #pragma mark -

// ImportBrush
BaseObjectRef
WonderBrush2Importer::ImportBrush(const BMessage& archive) const
{
	Brush* brush = new(std::nothrow) Brush();
	if (brush != NULL) {
		brush->Unarchive(&archive);
		_RestoreBaseObject(brush, archive);
	}
	return BaseObjectRef(brush, true);
}

// ImportPath
BaseObjectRef
WonderBrush2Importer::ImportPath(const BMessage& archive) const
{
	Path* path = new(std::nothrow) Path(&archive);
	if (path != NULL)
		_RestoreBaseObject(path, archive);
	return BaseObjectRef(path, true);
}

// ImportStrokeProperties
BaseObjectRef
WonderBrush2Importer::ImportStrokeProperties(const BMessage& archive) const
{
	StrokeProperties* strokeProperties = new(std::nothrow) StrokeProperties();

	if (strokeProperties != NULL) {
		float width = 0.0f;
		if (archive.FindFloat("width", &width) == B_OK)
			strokeProperties->SetWidth(width);
	
		float miterLimit = 0.0f;
		if (archive.FindFloat("miter limit", &miterLimit) == B_OK)
			strokeProperties->SetMiterLimit(miterLimit);
	
		int32 capMode = (int32)ButtCap;
		if (archive.FindInt32("cap mode", &capMode) == B_OK)
			strokeProperties->SetCapMode((CapMode)capMode);
	
		int32 joinMode = (int32)MiterJoin;
		if (archive.FindInt32("join mode", &joinMode) == B_OK)
			strokeProperties->SetJoinMode((JoinMode)joinMode);
	
		int32 position = (int32)CenterStroke;
		if (archive.FindInt32("position", &position) == B_OK)
			strokeProperties->SetStrokePosition((StrokePosition)position);

		_RestoreBaseObject(strokeProperties, archive);
	}
	return BaseObjectRef(strokeProperties, true);
}

// ImportColorRenderer
BaseObjectRef
WonderBrush2Importer::ImportColorRenderer(const BMessage& archive) const
{
	Paint* paint = new(std::nothrow) Paint();
	if (paint != NULL) {
		const void* data;
		ssize_t size;
		if (archive.FindData("RGBColor", B_RGB_COLOR_TYPE, &data,
				&size) == B_OK && size == sizeof(rgb_color)) {
			rgb_color color = *(const rgb_color*)data;
			paint->SetColor(color);
		}
	}
	return BaseObjectRef(paint, true);
}

// ImportGradientRenderer
BaseObjectRef
WonderBrush2Importer::ImportGradientRenderer(const BMessage& archive) const
{
	Gradient gradient(true);
	
	// Gradient settings
	int32 type;
	if (archive.FindInt32("type", &type) == B_OK)
		gradient.SetType((Gradient::Type)type);

	int32 interpolation;
	if (archive.FindInt32("interpolation", &interpolation) == B_OK)
		gradient.SetInterpolation((Gradient::Interpolation)interpolation);

	bool inheritTransformation;
	if (archive.FindBool("inherit transformation",
			&inheritTransformation) == B_OK) {
		gradient.SetInheritTransformation(inheritTransformation);
	}
	
	// Color stops
	const void* data;
	ssize_t size;
	float offset;
	for (int32 i = 0;; i++) {
		if (archive.FindData("RGBColor", B_RGB_COLOR_TYPE, i, &data,
				&size) == B_OK && size == sizeof(rgb_color)
			&& archive.FindFloat("offset", i, &offset) == B_OK) {
			rgb_color color = *(const rgb_color*)data;
			gradient.AddColor(color, offset);
		} else
			break;
	}

	_RestoreTransformable(&gradient, archive);

	Paint* paint = new(std::nothrow) Paint(
		GradientRef(new(std::nothrow) Gradient(gradient), true)
	);

	return BaseObjectRef(paint, true);
}

// ImportEraseRenderer
BaseObjectRef
WonderBrush2Importer::ImportEraseRenderer(const BMessage& archive) const
{
	Paint* paint = new(std::nothrow) Paint();
	if (paint != NULL)
		paint->SetType(Paint::ERASE);
	return BaseObjectRef(paint, true);
}

// #pragma mark -

// ImportGlobalResources
template<class Type, class Container>
status_t
WonderBrush2Importer::_ImportObjects(const BMessage& archive,
	Container* container) const
{
	for (int32 i = 0;; i++) {
		BMessage objectArchive;
		status_t ret = archive.FindMessage("stroke", i, &objectArchive);
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
WonderBrush2Importer::_RestoreStyleable(Styleable* styleable,
	const BMessage& archive) const
{
	_RestoreBoundedObject(styleable, archive);

	BMessage rendererArchive;
	if (archive.FindMessage("renderer", &rendererArchive) != B_OK)
		return;
	
	StyleRef styleRef = _ImportStyle(rendererArchive);
	if (styleRef.Get() != NULL)
		styleable->SetStyle(styleRef.Get());
}

// _RestoreBoundedObject
void
WonderBrush2Importer::_RestoreBoundedObject(BoundedObject* object,
	const BMessage& archive) const
{
	_RestoreObject(object, archive);
}

// _RestoreObject
void
WonderBrush2Importer::_RestoreObject(Object* object, const BMessage& archive) const
{
	_RestoreTransformable(object, archive);
	_RestoreBaseObject(object, archive);
}

// _RestoreBaseObject
void
WonderBrush2Importer::_RestoreBaseObject(BaseObject* object,
	const BMessage& archive) const
{
	BString name;
	if (archive.FindString("name", &name) == B_OK)
		object->SetName(name);
}

// _RestoreMatrix
void
WonderBrush2Importer::_RestoreTransformable(Transformable* transformable,
	const BMessage& archive) const
{
	Transformable t;
	double matrix[Transformable::MatrixSize];
	t.StoreTo(matrix);

	status_t ret = B_OK;

	for (int32 i = 0; i < Transformable::MatrixSize; i++) {
		status_t status = archive.FindDouble("affine matrix", i, &matrix[i]);
		if (i < 6) {
			// For compatibility with older WonderBrush documents that
			// did not store perspective transformations, errors are ignored
			// for matrix fields 6, 7, and 8.
			ret = status;
		}
		if (status != B_OK)
			break;
	}

	if (ret == B_OK) {
		t.sx = matrix[0];
		t.shy = matrix[1];
		t.shx = matrix[2];
		t.sy = matrix[3];
		t.tx = matrix[4];
		t.ty = matrix[5];

		t.w0 = matrix[6];
		t.w1 = matrix[7];
		t.w2 = matrix[8];
		
		transformable->SetTransformable(t);
	}
}

// _ImportStyle
StyleRef
WonderBrush2Importer::_ImportStyle(const BMessage& rendererArchive) const
{
	Style* style = new(std::nothrow) Style();
	if (style != NULL) {
		BaseObjectRef ref = ImportObject(rendererArchive);
		Paint* paint = dynamic_cast<Paint*>(ref.Get());
		if (paint != NULL) {
			style->SetFillPaint(PaintRef(paint));
			style->SetStrokePaint(PaintRef(new(std::nothrow)Paint(*paint),
				true));
		}
	}

	return StyleRef(style, true);
}
