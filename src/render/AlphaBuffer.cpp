/*
 * Copyright 2013 Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "AlphaBuffer.h"

#include <new>

#include <stdio.h>
#include <string.h>

#include <debugger.h>

// constructor
AlphaBuffer::AlphaBuffer(const BRect& bounds)
	: PixelBuffer(bounds, 2)
{
}

// constructor
AlphaBuffer::AlphaBuffer(uint32 width, uint32 height)
	: PixelBuffer(width, height, 2)
{
}

// constructor
AlphaBuffer::AlphaBuffer(AlphaBuffer* buffer, BRect area, bool adopt)
	: PixelBuffer(buffer, area, adopt)
{
}

// constructor
AlphaBuffer::AlphaBuffer(uint8* buffer, uint32 width, uint32 height,
		uint32 bytesPerRow, bool adopt)
	: PixelBuffer(buffer, width, height, 2, bytesPerRow, adopt)
{
}

// Attach
void
AlphaBuffer::Attach(uint8* buffer, uint32 width, uint32 height,
	uint32 bytesPerRow, bool adopt)
{
	_Attach(buffer, width, height, 2, bytesPerRow, adopt);
}
