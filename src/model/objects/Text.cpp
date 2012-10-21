/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "Text.h"

#include "FontCache.h"
#include "RenderEngine.h"
#include "TextSnapshot.h"

// constructor
Text::Text(const rgb_color& color)
	: Styleable(color)
	, fTextLayout(FontCache::getInstance())
{
	InitBounds();
}

// destructor
Text::~Text()
{
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Text::Snapshot() const
{
	return new TextSnapshot(this);
}

// DefaultName
const char*
Text::DefaultName() const
{
	return "Text";
}

// HitTest
bool
Text::HitTest(const BPoint& canvasPoint)
{
	RenderEngine engine(Transformation());
	return engine.HitTest(Bounds(), canvasPoint);
}

// Bounds
BRect
Text::Bounds()
{
	BRect bounds(0.0f, 0.0f, fTextLayout.getWidth(), fTextLayout.getHeight());
	Style()->ExtendBounds(bounds);
	return bounds;
}

// #pragma mark -

// SetWidth
void
Text::SetWidth(double width)
{
	if (width == fTextLayout.getWidth())
		return;

	fTextLayout.setWidth(width);

	UpdateChangeCounter();
	UpdateBounds();
}

// Width
double
Text::Width()
{
	return fTextLayout.getWidth();
}

// SetText
void
Text::SetText(const char* utf8String)
{
	fTextLayout.setText(utf8String);

	UpdateChangeCounter();
	UpdateBounds();
}

// getTextLayout
const TextLayout&
Text::getTextLayout() const
{
	return fTextLayout;
}

