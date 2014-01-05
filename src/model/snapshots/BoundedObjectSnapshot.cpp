/*
 * Copyright 2007 - 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "BoundedObjectSnapshot.h"

#include "BoundedObject.h"

// constructor
BoundedObjectSnapshot::BoundedObjectSnapshot(const BoundedObject* object)
	: ObjectSnapshot(object)
	, fOpacity(object->Opacity())
	, fOriginal(object)
{
}

// destructor
BoundedObjectSnapshot::~BoundedObjectSnapshot()
{
}

// #pragma mark -

// Sync
bool
BoundedObjectSnapshot::Sync()
{
	if (ObjectSnapshot::Sync()) {
		fOpacity = fOriginal->Opacity();
		return true;
	}
	return false;
}

// Layout
void
BoundedObjectSnapshot::Layout(LayoutContext& context, uint32 flags)
{
	// TODO: Opacity
	ObjectSnapshot::Layout(context, flags);
}


