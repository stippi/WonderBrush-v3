/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "Paint.h"

// constructor
Paint::Paint(const char* defaultName)
	:
	BaseObject(defaultName)
{
}

// constructor
Paint::Paint(const Paint& other)
	:
	BaseObject(other)
{
}

// constructor
Paint::Paint(BMessage* archive)
	:
	BaseObject(archive)
{
}

// destructor
Paint::~Paint()
{
}

// #pragma mark -

// Archive
status_t
Paint::Archive(BMessage* into, bool deep) const
{
	return BaseObject::Archive(into, deep);
}


