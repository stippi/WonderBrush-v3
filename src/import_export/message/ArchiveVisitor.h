/*
 * Copyright 2013-2014, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef ARCHIVE_VISITOR_H
#define ARCHIVE_VISITOR_H

#include "DocumentVisitor.h"

class BMessage;
class Color;
class ColorShade;
class Gradient;

class ArchiveVisitor : public DocumentVisitor<BMessage> {
	typedef DocumentVisitor<BMessage> inherited;
	
public:
								ArchiveVisitor(const DocumentRef& document,
									BMessage* archive);
	
	virtual	bool				VisitDocument(Document* document,
									BMessage* context);

	virtual	bool				VisitLayer(Layer* layer, BMessage* context);

	virtual	bool				VisitObject(Object* object, BMessage* context);

	virtual	bool				VisitBoundedObject(BoundedObject* boundedObject,
									BMessage* context);

	virtual	bool				VisitFilter(Filter* filter, BMessage* context);

	virtual	bool				VisitFilterBrightness(
									FilterBrightness* brightness,
									BMessage* context);

	virtual	bool				VisitFilterContrast(
									FilterContrast* contrast,
									BMessage* context);

	virtual	bool				VisitFilterDropShadow(
									FilterDropShadow* dropShadow,
									BMessage* context);

	virtual	bool				VisitFilterSaturation(
									FilterSaturation* saturation,
									BMessage* context);

	virtual	bool				VisitBrushStroke(BrushStroke* brushStroke,
									BMessage* context);

	virtual	bool				VisitImage(Image* image, BMessage* context);

	virtual	bool				VisitStyleable(Styleable* styleable,
									BMessage* context);

	virtual	bool				VisitRect(Rect* rect, BMessage* context);

	virtual	bool				VisitShape(Shape* shape, BMessage* context);

	virtual	bool				VisitText(Text* text, BMessage* context);

private:
			status_t			_StoreObject(Object* object,
									BMessage* archive) const;

			status_t			_StoreResources(const ResourceList& resources,
									BMessage* archive) const;
		
			status_t			_StoreResource(BaseObject* object,
									BMessage* archive) const;
		
			status_t			_StorePath(Path* path, BMessage* archive) const;
		
			status_t			_StoreStyle(Style* style,
									BMessage* archive) const;
		
			status_t			_StorePaint(Paint* paint,
									BMessage* archive) const;
		
			status_t			_StoreStrokeProperties(
									StrokeProperties* properties,
									BMessage* archive) const;
		
			status_t			_StoreColorProvider(ColorProvider* provider,
									BMessage* archive) const;
		
			status_t			_StoreColor(Color* color,
									BMessage* archive) const;
		
			status_t			_StoreColorShade(ColorShade* shade,
									BMessage* archive) const;
		
			status_t			_StoreGradient(const Gradient* gradient,
									BMessage* archive) const;
		
			status_t			_StoreBrush(Brush* brush,
									BMessage* archive) const;
		
			status_t			_StoreStyleRuns(Text* text,
									BMessage* archive) const;
			
			status_t			_StoreStyleRun(const StyleRun* run,
									BMessage* archive) const;
		
			status_t			_StoreResourceOrIndex(BaseObject* object,
									BMessage* archive) const;
		
			int32				_GlobalResourceIndex(BaseObject* object) const;

private:
			DocumentRef			fDocument;

public:
			status_t			status;
};

#endif // ARCHIVE_VISITOR_H
