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
#include "Font.h"
#include "Image.h"
#include "Layer.h"
#include "Object.h"
#include "Paint.h"
#include "Shape.h"
#include "StrokeProperties.h"
#include "Style.h"
#include "Styleable.h"
#include "StyleRun.h"
#include "StyleRunList.h"
#include "Rect.h"
#include "Text.h"
#include "ui_defines.h"

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
	Filter* filter = new(std::nothrow) Filter();
	if (filter != NULL) {
		float radius;
		if (archive.FindFloat("radius", &radius) == B_OK)
			filter->SetFilterRadius(radius);
		
		_RestoreObject(filter, archive);
	}
	return BaseObjectRef(filter, true);
}

// ImportFilterDropShadow
BaseObjectRef
MessageImporter::ImportFilterDropShadow(const BMessage& archive) const
{
	FilterDropShadow* filter = new(std::nothrow) FilterDropShadow();
	if (filter != NULL) {
		float value;
		if (archive.FindFloat("radius", &value) == B_OK)
			filter->SetFilterRadius(value);
		if (archive.FindFloat("offset-x", &value) == B_OK)
			filter->SetOffsetX(value);
		if (archive.FindFloat("offset-y", &value) == B_OK)
			filter->SetOffsetY(value);
		if (archive.FindFloat("opacity", &value) == B_OK)
			filter->SetOpacity(value);

		BMessage colorArchive;
		if (archive.FindMessage("color", &colorArchive) == B_OK) {
			BaseObjectRef colorRef = ImportObject(colorArchive);
			ColorProvider* color = dynamic_cast<ColorProvider*>(colorRef.Get());
			if (color != NULL)
				filter->SetColor(ColorProviderRef(color));
		}

		_RestoreObject(filter, archive);
	}
	return BaseObjectRef(filter, true);
}

// ImportFilterSaturation
BaseObjectRef
MessageImporter::ImportFilterSaturation(const BMessage& archive) const
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
MessageImporter::ImportImage(const BMessage& archive) const
{
	// TODO
	return BaseObjectRef();
}

// ImportLayer
BaseObjectRef
MessageImporter::ImportLayer(const BMessage& archive) const
{
	Layer* layer = new(std::nothrow) Layer(fDocument->Bounds());
	if (layer != NULL) {
		ImportObjects(archive, layer);

		_RestoreObject(layer, archive);
	}
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
	Text* text = new(std::nothrow) Text(kBlack);
	if (text != NULL) {
		BString string;
		status_t ret = archive.FindString("text", &string);

		if (ret == B_OK) {
			double width;
			ret = archive.FindDouble("width", &width);
			if (ret == B_OK)
				text->SetWidth(width);
		}

		int32 length = 0;
		StyleRunList styleRuns;
		if (ret == B_OK) {
			BMessage runArchive;
			for (int32 i = 0;; i++) {
				if (archive.FindMessage("style run", i, &runArchive) != B_OK)
					break;

				int32 runLength;
				ret = runArchive.FindInt32("length", &runLength);

				const char* fontFamily;
				if (ret == B_OK)
					ret = runArchive.FindString("font family", &fontFamily);

				const char* fontStyle;
				if (ret == B_OK)
					ret = runArchive.FindString("font style", &fontStyle);

				double fontSize;
				if (ret == B_OK)
					ret = runArchive.FindDouble("font size", &fontSize);

				int32 scriptLevel = Font::NORMAL;
				runArchive.FindInt32("font script level", &scriptLevel);

				double glyphSpacing = 0.0;
				runArchive.FindDouble("glyph spacing", &glyphSpacing);

				double fauxWeight = 0.0;
				runArchive.FindDouble("faux weight", &fauxWeight);

				double fauxItalic = 0.0;
				runArchive.FindDouble("faux italic", &fauxItalic);
				
				BMessage styleArchive;
				BaseObjectRef styleRef;
				if (ret == B_OK) {
					ret = runArchive.FindMessage("style", &styleArchive);
					styleRef = ImportStyle(styleArchive);
				}
				Style* style = dynamic_cast<Style*>(styleRef.Get());
				
				if (ret == B_OK && style != NULL) {
					Font font(fontFamily, fontStyle, fontSize,
						(Font::ScriptLevel)scriptLevel);
					CharacterStyle* characterStyle
						= new(std::nothrow) CharacterStyle(
							font, glyphSpacing, fauxWeight, fauxItalic,
							StyleRef(style)
						);
					if (characterStyle == NULL)
						ret = B_NO_MEMORY;
					else {
						StyleRun run(CharacterStyleRef(characterStyle, true));
						run.SetLength(runLength);
						if (!styleRuns.Append(run))
							ret = B_NO_MEMORY;
						else
							length += runLength;
					}
				} else {
					fprintf(stderr, "MessageImporter::ImportText() - "
						"Failed to restore CharacterStyle!\n");
				}
			}
		}

		if (ret == B_OK && length == string.CountChars())
			text->Insert(0, string, styleRuns);
	
		_RestoreStyleable(text, archive);
	}
	return BaseObjectRef(text, true);
}

// #pragma mark -

// ImportBrush
BaseObjectRef
MessageImporter::ImportBrush(const BMessage& archive) const
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
MessageImporter::ImportColor(const BMessage& archive) const
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
MessageImporter::ImportColorShade(const BMessage& archive) const
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
				fprintf(stderr, "MessageImporter::ImportColorShade() - "
					"Failed to restore ColorProvider!\n");
			}
		}
	
		_RestoreBaseObject(colorShade, archive);
	}
	return BaseObjectRef(colorShade, true);
}

// ImportGradient
BaseObjectRef
MessageImporter::ImportGradient(const BMessage& archive) const
{
	// TODO
	return BaseObjectRef();
}

// ImportPaint
BaseObjectRef
MessageImporter::ImportPaint(const BMessage& archive) const
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
				fprintf(stderr, "MessageImporter::ImportPaint() - "
					"Failed to restore ColorProvider!\n");
			}
		} else {
			fprintf(stderr, "MessageImporter::ImportPaint() - "
				"unkown Paint type!\n");
		}
		
		_RestoreBaseObject(paint, archive);
	}
	return BaseObjectRef(paint, true);
}

// ImportPath
BaseObjectRef
MessageImporter::ImportPath(const BMessage& archive) const
{
	Path* path = new(std::nothrow) Path(&archive);
	if (path != NULL)
		_RestoreBaseObject(path, archive);
	return BaseObjectRef(path, true);
}

// ImportStrokeProperties
BaseObjectRef
MessageImporter::ImportStrokeProperties(const BMessage& archive) const
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

// ImportStyle
BaseObjectRef
MessageImporter::ImportStyle(const BMessage& archive) const
{
	Style* style = new(std::nothrow) Style();
	if (style != NULL) {
		BMessage fillPaintArchive;
		if (archive.FindMessage("fill paint", &fillPaintArchive) == B_OK) {
			BaseObjectRef ref = ImportObject(fillPaintArchive);
			Paint* paint = dynamic_cast<Paint*>(ref.Get());
			if (paint != NULL)
				style->SetFillPaint(*paint);
		}

		BMessage strokePaintArchive;
		if (archive.FindMessage("stroke paint", &strokePaintArchive) == B_OK) {
			BaseObjectRef ref = ImportObject(strokePaintArchive);
			Paint* paint = dynamic_cast<Paint*>(ref.Get());
			if (paint != NULL)
				style->SetStrokePaint(*paint);
		}

		BMessage strokePropertiesArchive;
		if (archive.FindMessage("stroke paint",
				&strokePropertiesArchive) == B_OK) {
			BaseObjectRef ref = ImportObject(strokePropertiesArchive);
			StrokeProperties* strokeProperties
				= dynamic_cast<StrokeProperties*>(ref.Get());
			if (strokeProperties != NULL)
				style->SetStrokeProperties(*strokeProperties);
		}

		_RestoreBaseObject(style, archive);
	}
	return BaseObjectRef(style, true);
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
	_RestoreBoundedObject(styleable, archive);

	BMessage styleArchive;
	if (archive.FindMessage("style", &styleArchive) != B_OK)
		return;
	
	BaseObjectRef styleRef = ImportObject(styleArchive);
	Style* style = dynamic_cast<Style*>(styleRef.Get());
	if (style != NULL)
		styleable->SetStyle(style);
}

// _RestoreBoundedObject
void
MessageImporter::_RestoreBoundedObject(BoundedObject* object,
	const BMessage& archive) const
{
	_RestoreObject(object, archive);
}

// _RestoreObject
void
MessageImporter::_RestoreObject(Object* object, const BMessage& archive) const
{
	const double* matrix;
	ssize_t size;
	if (archive.FindData("matrix", B_DOUBLE_TYPE,
			(const void**)&matrix, &size) == B_OK
		&& size == Transformable::MatrixSize * sizeof(double)) {
		object->LoadFrom(matrix);
	}

	_RestoreBaseObject(object, archive);
}

// _RestoreBaseObject
void
MessageImporter::_RestoreBaseObject(BaseObject* object,
	const BMessage& archive) const
{
	BString name;
	if (archive.FindString("name", &name) == B_OK)
		object->SetName(name);
}

