/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "ImageSnapshot.h"

#include <stdio.h>

#include "Image.h"
#include "RenderBuffer.h"
#include "RenderEngine.h"

// constructor
ImageSnapshot::ImageSnapshot(const Image* image)
	: ObjectSnapshot(image)
	, fOriginal(image)
	, fBuffer(image->Buffer())
{
	if (fBuffer != NULL)
		fBuffer->AddReference();
}

// destructor
ImageSnapshot::~ImageSnapshot()
{
	if (fBuffer != NULL)
		fBuffer->RemoveReference();
}

// #pragma mark -

// Original
const Object*
ImageSnapshot::Original() const
{
	return fOriginal;
}

// Sync
bool
ImageSnapshot::Sync()
{
	return ObjectSnapshot::Sync();
}

// Render
void
ImageSnapshot::Render(RenderEngine& engine, RenderBuffer* bitmap,
	BRect area) const
{
	if (fBuffer != NULL) {
		engine.SetTransformation(LayoutedState().Matrix);
		engine.DrawImage(fBuffer, area);
	}
}


