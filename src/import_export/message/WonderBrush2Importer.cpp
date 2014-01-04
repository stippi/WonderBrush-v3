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
#include "Style.h"
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
		if (layer == NULL || !fDocument->RootLayer()->AddObject(layer))
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

	if (type == "Gradient")
		return ImportGradient(archive);

	if (type == "Paint")
		return ImportPaint(archive);

	if (type == "VectorPath")
		return ImportPath(archive);

	if (type == "StrokeProperties")
		return ImportStrokeProperties(archive);

	if (type == "ColorRenderer")
		return ImportColorRenderer(archive);

	if (type == "GradientRenderer")
		return ImportGradientRenderer(archive);

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
		BMessage brushArchive;
		if (archive.FindMessage("brush", &brushArchive) == B_OK) {
			BaseObjectRef ref = ImportObject(brushArchive);
			Brush* brush = dynamic_cast<Brush*>(ref.Get());
			if (brush != NULL)
				brushStroke->SetBrush(brush);
		}

		BMessage paintArchive;
		if (archive.FindMessage("paint", &paintArchive) == B_OK) {
			BaseObjectRef ref = ImportObject(paintArchive);
			Paint* paint = dynamic_cast<Paint*>(ref.Get());
			if (paint != NULL)
				brushStroke->SetPaint(paint);
		}

		BMessage strokeArchive;
		if (archive.FindMessage("stroke", &strokeArchive) == B_OK) {
			for (int32 i = 0;; i++) {
				BPoint point;
				float pressure;
				float tiltX;
				float tiltY;
				if (strokeArchive.FindPoint("point", i, &point) != B_OK
					|| strokeArchive.FindFloat("pressure", i,
						&pressure) != B_OK
					|| strokeArchive.FindFloat("tilt-x", i, &tiltX) != B_OK
					|| strokeArchive.FindFloat("tilt-y", i, &tiltY) != B_OK
					|| !brushStroke->AppendPoint(
							StrokePoint(point, pressure, tiltX, tiltY))) {
					break;
				}
			}
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
		// TODO: int32 "flags" (??)

		_RestoreObject(layer, archive);
	}
	return BaseObjectRef(layer, true);
}

// ImportRect
BaseObjectRef
WonderBrush2Importer::ImportRect(const BMessage& archive) const
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
			BaseObjectRef ref = ImportColorRenderer(styleArchive);
			Style* style = dynamic_cast<Style*>(ref.Get());
			if (ret == B_OK && style != NULL) {
				Font font(fontFamily, fontStyle, fontSize * 16.0f);
				text->SetText(string, font, StyleRef(style));
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

// ImportColor
BaseObjectRef
WonderBrush2Importer::ImportColor(const BMessage& archive) const
{
	rgb_color rgba = kBlack;
	archive.FindUInt8("r", &rgba.red);
	archive.FindUInt8("g", &rgba.green);
	archive.FindUInt8("b", &rgba.blue);
	archive.FindUInt8("a", &rgba.alpha);
	Color* color = new(std::nothrow) Color(rgba);
	if (color != NULL)
		_RestoreBaseObject(color, archive);
	return BaseObjectRef(color, true);
}

// ImportColorShade
BaseObjectRef
WonderBrush2Importer::ImportColorShade(const BMessage& archive) const
{
	ColorShade* colorShade = new(std::nothrow) ColorShade();
	if (colorShade != NULL) {
		float value;
		if (archive.FindFloat("h", &value) == B_OK)
			colorShade->SetHue(value);
		if (archive.FindFloat("s", &value) == B_OK)
			colorShade->SetSaturation(value);
		if (archive.FindFloat("v", &value) == B_OK)
			colorShade->SetValue(value);
	
		BMessage providerArchive;
		if (archive.FindMessage("provider", &providerArchive) == B_OK) {
			BaseObjectRef ref = ImportObject(providerArchive);
			ColorProvider* provider = dynamic_cast<ColorProvider*>(ref.Get());
			if (provider != NULL)
				colorShade->SetColorProvider(ColorProviderRef(provider));
			else {
				fprintf(stderr, "WonderBrush2Importer::ImportColorShade() - "
					"Failed to restore ColorProvider!\n");
			}
		}
	
		_RestoreBaseObject(colorShade, archive);
	}
	return BaseObjectRef(colorShade, true);
}

// ImportGradient
BaseObjectRef
WonderBrush2Importer::ImportGradient(const BMessage& archive) const
{
	// TODO
	return BaseObjectRef();
}

// ImportPaint
BaseObjectRef
WonderBrush2Importer::ImportPaint(const BMessage& archive) const
{
	Paint* paint = new(std::nothrow) Paint();
	if (paint != NULL) {
		// try color
		BMessage colorArchive;
		if (archive.FindMessage("color", &colorArchive) == B_OK) {
			BaseObjectRef ref = ImportObject(colorArchive);
			ColorProvider* provider = dynamic_cast<ColorProvider*>(ref.Get());
			if (provider != NULL)
				paint->SetColorProvider(ColorProviderRef(provider));
			else {
				fprintf(stderr, "WonderBrush2Importer::ImportPaint() - "
					"Failed to restore ColorProvider!\n");
			}
		} else {
			fprintf(stderr, "WonderBrush2Importer::ImportPaint() - "
				"unkown Paint type!\n");
		}
		
		_RestoreBaseObject(paint, archive);
	}
	return BaseObjectRef(paint, true);
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
	Style* style = new(std::nothrow) Style();
	if (style != NULL) {
		const void* data;
		ssize_t size;
		if (archive.FindData("RGBColor", B_RGB_COLOR_TYPE, &data,
				&size) == B_OK && size == sizeof(rgb_color)) {
			rgb_color color = *(const rgb_color*)data;
			style->SetFillPaint(PaintRef(new(std::nothrow) Paint(color), true));
			style->SetStrokePaint(PaintRef(new(std::nothrow) Paint(color), true));
		}
	}
	return BaseObjectRef(style, true);
}

// ImportGradientRenderer
BaseObjectRef
WonderBrush2Importer::ImportGradientRenderer(const BMessage& archive) const
{
	Style* style = new(std::nothrow) Style();
	if (style != NULL) {
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

		style->SetFillPaint(PaintRef(
			new(std::nothrow) Paint(
				GradientRef(new(std::nothrow) Gradient(gradient), true)
			), true)
		);
		style->SetStrokePaint(PaintRef(
			new(std::nothrow) Paint(
				GradientRef(new(std::nothrow) Gradient(gradient), true)
			), true)
		);
	}
	return BaseObjectRef(style, true);
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
	
	BaseObjectRef styleRef = ImportObject(rendererArchive);
	Style* style = dynamic_cast<Style*>(styleRef.Get());
	if (style != NULL)
		styleable->SetStyle(style);
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
