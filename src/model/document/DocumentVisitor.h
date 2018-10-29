/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef DOCUMENT_VISITOR_H
#define DOCUMENT_VISITOR_H

#include "BoundedObject.h"
#include "BrushStroke.h"
#include "Document.h"
#include "Filter.h"
#include "FilterBrightness.h"
#include "FilterDropShadow.h"
#include "FilterSaturation.h"
#include "Image.h"
#include "Layer.h"
#include "Object.h"
#include "Styleable.h"
#include "Shape.h"
#include "Rect.h"
#include "Text.h"

template <typename Context>
class DocumentVisitor {
public:
	virtual bool VisitDocument(Document* document, Context* context)
	{
		return VisitLayer(document->RootLayer(), context);
	}

	virtual bool VisitLayer(Layer* layer, Context* context)
	{
		int count = layer->CountObjects();
		for (int i = 0; i < count; i++) {
			Object* object = layer->ObjectAtFast(i);
			Layer* subLayer = dynamic_cast<Layer*>(object);
			if (subLayer != NULL) {
				if (!VisitLayer(subLayer, context))
					return false;
			} else {
				if (!VisitObject(object, context))
					return false;
			}
		}
		return true;
	}

	virtual bool VisitObject(Object* object, Context* context)
	{
		// Determine type of Object
		BoundedObject* boundedObject = dynamic_cast<BoundedObject*>(object);
		if (boundedObject != NULL)
			return VisitBoundedObject(boundedObject, context);
	
		Filter* filter = dynamic_cast<Filter*>(object);
		if (filter != NULL)
			return VisitFilter(filter, context);
		
		FilterBrightness* brightness = dynamic_cast<FilterBrightness*>(object);
		if (brightness != NULL)
			return VisitFilterBrightness(brightness, context);
	
		FilterDropShadow* dropShadow = dynamic_cast<FilterDropShadow*>(object);
		if (dropShadow != NULL)
			return VisitFilterDropShadow(dropShadow, context);
	
		FilterSaturation* saturation = dynamic_cast<FilterSaturation*>(object);
		if (saturation != NULL)
			return VisitFilterSaturation(saturation, context);
	
		return true;
	}

	virtual bool VisitBoundedObject(BoundedObject* boundedObject,
		Context* context)
	{
		// Determine type of BoundedObject
		Styleable* styleable = dynamic_cast<Styleable*>(boundedObject);
		if (styleable != NULL)
			return VisitStyleable(styleable, context);
	
		BrushStroke* brushStroke = dynamic_cast<BrushStroke*>(boundedObject);
		if (brushStroke != NULL)
			return VisitBrushStroke(brushStroke, context);
		
		Image* image = dynamic_cast<Image*>(boundedObject);
		if (image != NULL)
			return VisitImage(image, context);
	
		return true;
	}

	virtual bool VisitFilter(Filter* filter, Context* context)
	{
		return true;
	}

	virtual bool VisitFilterBrightness(FilterBrightness* brightness,
		Context* context)
	{
		return true;
	}

	virtual bool VisitFilterDropShadow(FilterDropShadow* dropShadow,
		Context* context)
	{
		return true;
	}

	virtual bool VisitFilterSaturation(FilterSaturation* saturation,
		Context* context)
	{
		return true;
	}

	virtual bool VisitBrushStroke(BrushStroke* stroke, Context* context)
	{
		return true;
	}

	virtual bool VisitImage(Image* image, Context* context)
	{
		return true;
	}

	virtual bool VisitStyleable(Styleable* stylable, Context* context)
	{
		// Determine type of Styleable
		Rect* rect = dynamic_cast<Rect*>(stylable);
		if (rect != NULL)
			return VisitRect(rect, context);
	
		Shape* shape = dynamic_cast<Shape*>(stylable);
		if (shape != NULL)
			return VisitShape(shape, context);
		
		Text* text = dynamic_cast<Text*>(stylable);
		if (text != NULL)
			return VisitText(text, context);
	
		return true;
	}

	virtual bool VisitRect(Rect* rect, Context* context)
	{
		return true;
	}

	virtual bool VisitShape(Shape* shape, Context* context)
	{
		return true;
	}

	virtual bool VisitText(Text* text, Context* context)
	{
		return true;
	}
};

#endif // DOCUMENT_VISITOR_H
