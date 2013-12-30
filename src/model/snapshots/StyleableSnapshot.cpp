/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "StyleableSnapshot.h"

#include <stdio.h>

#include "AutoLocker.h"
#include "Paint.h"
	// TODO: Remove, put all the handling for Style into RenderEngine...
#include "Style.h"
#include "Styleable.h"

// constructor
StyleableSnapshot::StyleableSnapshot(const Styleable* styleable)
	: ObjectSnapshot(styleable)
	, fOriginal(styleable)
	, fFillPaint(NULL)
	, fStrokePaint(NULL)
	, fStrokeProperties(NULL)
{
//	SetFillPaint(Paint::EmptyPaint());
//	SetStrokePaint(Paint::EmptyPaint());
//	SetStrokeProperties(::StrokeProperties());
	_SyncStyle();
}

// destructor
StyleableSnapshot::~StyleableSnapshot()
{
	_UnsetFillPaint();
	_UnsetStrokePaint();
	_UnsetStrokeProperties();
}

// #pragma mark -

// Sync
bool
StyleableSnapshot::Sync()
{
	if (ObjectSnapshot::Sync()) {
		_SyncStyle();
		return true;
	}
	return false;
}

// Render
void
StyleableSnapshot::PrepareRenderEngine(RenderEngine& engine) const
{
	engine.SetFillPaint(fFillPaint);
	engine.SetStrokePaint(fStrokePaint);
	engine.SetStrokeProperties(fStrokeProperties);
}

// _SyncStyle
void
StyleableSnapshot::_SyncStyle()
{
	Style* style = fOriginal->Style();

	if (style->FillPaint() != NULL)
		_SetFillPaint(*style->FillPaint());
	else
		_UnsetFillPaint();

	if (style->StrokePaint() != NULL)
		_SetStrokePaint(*style->StrokePaint());
	else
		_UnsetStrokePaint();

	if (style->StrokeProperties() != NULL)
		_SetStrokeProperties(*style->StrokeProperties());
	else
		_UnsetStrokeProperties();
}

// _SetFillPaint
void
StyleableSnapshot::_SetFillPaint(const Paint& paint)
{
	_SetProperty(fFillPaint, paint, Paint::PaintCache());
}

// _UnsetFillPaint
void
StyleableSnapshot::_UnsetFillPaint()
{
	_UnsetProperty(fFillPaint, Paint::PaintCache());
}

// _SetStrokePaint
void
StyleableSnapshot::_SetStrokePaint(const Paint& paint)
{
	_SetProperty(fStrokePaint, paint, Paint::PaintCache());
}

// _UnsetStrokePaint
void
StyleableSnapshot::_UnsetStrokePaint()
{
	_UnsetProperty(fStrokePaint, Paint::PaintCache());
}

// _SetStrokeProperties
void
StyleableSnapshot::_SetStrokeProperties(const ::StrokeProperties& properties)
{
	_SetProperty(fStrokeProperties, properties,
		StrokeProperties::StrokePropertiesCache());
}

// _UnsetStrokeProperties
void
StyleableSnapshot::_UnsetStrokeProperties()
{
	_UnsetProperty(fStrokeProperties,
		StrokeProperties::StrokePropertiesCache());
}

// _SetProperty
template<typename PropertyType, typename ValueType, typename CacheType>
void
StyleableSnapshot::_SetProperty(PropertyType*& member,
	const ValueType& newValue, CacheType& cache)
{
	if (member == NULL) {
		member = cache.Get(newValue);
		return;
	}

	if (*member == newValue)
		return;

	member = cache.PrepareForModifications(member);
	*member = newValue;
	member = cache.CommitModifications(member);
}

// _UnsetProperty
template<typename PropertyType, typename CacheType>
void
StyleableSnapshot::_UnsetProperty(PropertyType*& member, CacheType& cache)
{
	if (member == NULL)
		return;

	cache.Put(member);
	member = NULL;
}
