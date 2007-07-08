/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "ObjectSnapshot.h"

#include "Object.h"

// constructor
ObjectSnapshot::ObjectSnapshot(const Object* object)
	: Transformable(object->Transformation())
	, fChangeCounter(object->ChangeCounter())
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

	SetTransformable(Original()->Transformation());
	fChangeCounter = Original()->ChangeCounter();
	return true;
}

// PrepareRendering
void
ObjectSnapshot::PrepareRendering(BRect documentBounds)
{
	// do anything necessary to invoke a rendering
	// for example, a shape might pre-rasterize, but
	// it needs to protect this step by a lock, because
	// PrepareRendering() is executed by each rendering
	// thread. Whichever thread executes first, should
	// do the work for all other threads, so that these
	// threads can quickly skip the preparation after
	// they have acquired the lock and see that the work
	// has already been done
}

// Render
void
ObjectSnapshot::Render(BBitmap* bitmap, BRect area) const
{
	// "area" is the area previously given in
	// RebuildAreaForDirtyArea()
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
	// "area" is the area requested to be rendered by this
	// object.
	// This function should change the area so that
	// it includes all pixels outside the given area which
	// are required by this object to render the given area
	// correctly.
}


