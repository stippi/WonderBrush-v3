/*
 * Copyright (c) 2009-2010 Stephan Aßmus <superstippi@gmx.de>
 */
//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//		    mcseemagg@yahoo.com
//		    http://www.antigrain.com
//----------------------------------------------------------------------------
//
// Class scanline_p - a general purpose scanline container with packed spans.
//
//----------------------------------------------------------------------------
//
// Adaptation for 32-bit screen coordinates (scanline32_p) has been sponsored by 
// Liberty Technology Systems, Inc., visit http://lib-sys.com
//
// Liberty Technology Systems, Inc. is the provider of
// PostScript and PDF technology for software developers.
// 
//----------------------------------------------------------------------------
#ifndef SCANLINE_H
#define SCANLINE_H

#include <stdio.h>
#include <string.h>

#include "DataBlock.h"

typedef uint8					CoverType;
typedef int16					CoordType;

struct Span {
	CoordType			x;
	CoordType			len; // If negative, it's a solid Span, covers is valid
	const CoverType*	covers;
};

typedef DataBlock<CoverType>	CoverAllocator;
typedef DataBlock<Span>			SpanAllocator;

//=============================================================Scanline
// 
// This is a general purpose scaline container which supports the interface 
// used in the rasterizer::render(). See description of scanline_u8
// for details.
// 
//------------------------------------------------------------------------
class Scanline {
public:
	typedef Scanline	SelfType;
	typedef Span*		iterator;
	typedef const Span*	const_iterator;

	~Scanline()
	{
	}

	Scanline()
		:
		fLastX(0x7FFFFFF0),
		fCovers(NULL),
		fCoverPtr(NULL),
		fSpans(NULL),
		fCurrentSpan(NULL),
		fSpanOffset(0),
		fCoverOffset(0),
		fCoverAllocator(NULL),
		fSpanAllocator(NULL)
	{
	}

	void SetAllocators(CoverAllocator* coverAllocator,
		SpanAllocator* spanAllocator)
	{
		fCoverAllocator = coverAllocator;
		fSpanAllocator = spanAllocator;
	}

	bool IsValid() const
	{
		return fCovers != NULL && fSpans != NULL;
	}

	void reset(int minX, int maxX)
	{
		unsigned maxLength = maxX - minX + 3;

		fCovers = fCoverAllocator->Reserve(maxLength);
		fSpans = fSpanAllocator->Reserve(maxLength) ;

		fLastX = 0x7FFFFFF0;
		fCoverPtr = fCovers;
		fCurrentSpan = fSpans;
		fCurrentSpan->len = 0;
	}

	void add_cell(int x, unsigned cover)
	{
		*fCoverPtr = (CoverType)cover;
		if (x == fLastX + 1 && fCurrentSpan->len > 0) {
			fCurrentSpan->len++;
		} else {
			fCurrentSpan++;
			fCurrentSpan->covers = fCoverPtr;
			fCurrentSpan->x = (CoordType)x;
			fCurrentSpan->len = 1;
		}
		fLastX = x;
		fCoverPtr++;
	}

	void add_cells(int x, unsigned len, const CoverType* covers)
	{
		memcpy(fCoverPtr, covers, len * sizeof(CoverType));
		if (x == fLastX + 1 && fCurrentSpan->len > 0) {
			fCurrentSpan->len += (CoordType)len;
		} else {
			fCurrentSpan++;
			fCurrentSpan->covers = fCoverPtr;
			fCurrentSpan->x = (CoordType)x;
			fCurrentSpan->len = (CoordType)len;
		}
		fCoverPtr += len;
		fLastX = x + len - 1;
	}

	void add_span(int x, unsigned len, unsigned cover)
	{
		if (x == fLastX + 1 && 
		   fCurrentSpan->len < 0 && 
		   cover == *fCurrentSpan->covers) {
			fCurrentSpan->len -= (CoordType)len;
		} else {
			*fCoverPtr = (CoverType)cover;
			fCurrentSpan++;
			fCurrentSpan->covers = fCoverPtr++;
			fCurrentSpan->x = (CoordType)x;
			fCurrentSpan->len = (CoordType)(-int(len));
		}
		fLastX = x + len - 1;
	}

	void finalize(int y) 
	{ 
		fY = y; 

		// Reserve only what we need from the allocators.
		// Remember the data offsets, since later spans may reallocate
		// memory so that our pointers become invalid!
		uint32 coverSize = fCoverPtr - fCovers + 1;
		if (fCoverAllocator->Reserve(fCovers, coverSize) != B_OK) {
			fCovers = NULL;
			fCoverPtr = NULL;
			fCoverOffset = 0;
		} else {
			fCoverOffset = fCovers - fCoverAllocator->DataAt(0);
		}

		uint32 spanSize = fCurrentSpan - fSpans + 1;
		if (fSpanAllocator->Reserve(fSpans, spanSize) != B_OK) {
			fSpans = NULL;
			fCurrentSpan = NULL;
			fSpanOffset = 0;
		} else {
			fSpanOffset = fSpans - fSpanAllocator->DataAt(0);
		}
	}

	void reset_spans()
	{
		fLastX	= 0x7FFFFFF0;
		fCoverPtr = fCovers;
		fCurrentSpan = fSpans;
		fCurrentSpan->len = 0;
	}

	void Validate()
	{
		// Validate is necessary because the cover and span allocators may have had
		// to relocate their memory block. This would make all our internal memory
		// pointers invalid, so we check adjust for the relocation here.
		if (fCovers == NULL || fSpans == NULL)
			return;

		CoverType* covers = fCoverAllocator->DataAt(fCoverOffset);
		Span* span = fSpanAllocator->DataAt(fSpanOffset);
		if (fCovers != covers || fSpans != span) {
			// We need to relocate the pointers!
			int coverOffset = covers - fCovers;
			unsigned len = num_spans();
			fSpans = span;
			if (coverOffset != 0) {
				fCurrentSpan = fSpans + 1;
				fCovers += coverOffset;
				fCoverPtr += coverOffset;
				while (len-- > 0) {
					if (fCurrentSpan->covers != NULL)
						fCurrentSpan->covers += coverOffset;
					fCurrentSpan++;
				}
				fCurrentSpan--;
			} else {
				fCurrentSpan = fSpans + len;
			}
		}
	}

	int y() const 					{ return fY; }
	unsigned num_spans() const		{ return unsigned(fCurrentSpan - fSpans); }
	const_iterator begin() const	{ return fSpans + 1; }

private:
	Scanline(const SelfType&);
	const SelfType& operator=(const SelfType& other);

	int		 			fLastX;
	int		 			fY;
	CoverType*			fCovers;
	CoverType*			fCoverPtr;
	Span*				fSpans;
	Span*				fCurrentSpan;

	uint32				fSpanOffset;
	uint32				fCoverOffset;

	CoverAllocator*		fCoverAllocator;
	SpanAllocator*		fSpanAllocator;
};

#endif // SCANLINE_H

