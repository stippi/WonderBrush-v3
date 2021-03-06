/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef BITMAP_SUPPORT_H
#define BITMAP_SUPPORT_H


#include <GraphicsDefs.h>
#include <Rect.h>


class BBitmap;

void	clear_area(const BBitmap* bitmap, rgb_color color, BRect area);

void	copy_area(const BBitmap* source, const BBitmap* dest, BRect area);

void	blend_area(const BBitmap* source, const BBitmap* dest, BRect area);

void	demultiply_area(const BBitmap* bitmap, BRect area);

BBitmap* scale_bitmap(const BBitmap* bitmap, BRect newBounds);


#endif // BITMAP_SUPPORT_H
