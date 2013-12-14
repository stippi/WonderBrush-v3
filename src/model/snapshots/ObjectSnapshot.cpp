/*
 * Copyright 2007 - 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "ObjectSnapshot.h"

#include "Object.h"

// constructor
ObjectSnapshot::ObjectSnapshot(const Object* object)
	: Transformable(object->LocalTransformation())
	, fChangeCounter(object->ChangeCounter())
	, fName(object->Name())
{
}

// destructor
ObjectSnapshot::~ObjectSnapshot()
{
}

// #pragma mark -

// Sync
bool
ObjectSnapshot::Sync()
{
	if (Original()->ChangeCounter() == fChangeCounter)
		return false;

	SetTransformable(Original()->LocalTransformation());
	fChangeCounter = Original()->ChangeCounter();
	fName = Original()->Name();
	return true;
}

// Layout
void
ObjectSnapshot::Layout(LayoutContext& context, uint32 flags)
{
	// TODO: Keep Transformable as a member, don't inherit it.
	context.SetTransformation(*this);
	fLayoutedState = *context.State();
}

// PrepareRendering
void
ObjectSnapshot::PrepareRendering(BRect documentBounds)
{
	// Do anything necessary to invoke a rendering
	// for example, a shape might pre-rasterize, but
	// it needs to protect this step by a lock, because
	// PrepareRendering() is executed by each rendering
	// thread. Whichever thread executes first, should
	// do the work for all other threads, so that these
	// threads can quickly skip the preparation after
	// they have acquired the lock and see that the work
	// has already been done.
}

// Render
void
ObjectSnapshot::Render(RenderEngine& engine, RenderBuffer* bitmap,
	BRect area) const
{
	// "Area" is the area previously given in
	// RebuildAreaForDirtyArea().
	// The object is requested to correctly produce all pixels
	// in the given area, where pixels outside this area can
	// be assumed to be valid, provided that
	// RebuildAreaForDirtyArea() previously returned these pixels
	// by extending this area.
}

// RebuildAreaForDirtyArea
void
ObjectSnapshot::RebuildAreaForDirtyArea(BRect& area) const
{
	// "Area" is the area requested to be rendered by this object.
	// This function should change the area so that
	// it includes all pixels outside the given area which
	// are required by this object to render the given area
	// correctly. For example, a filter may need pixels outside
	// "area" to compute the pixels inside "area". Thus, it should
	// extend "area" accordingly. During the render pass, this
	// object will be asked to produce all pixels within the
	// original area again, but it can assume the surface pixels
	// outside that area, to be valid.
}


