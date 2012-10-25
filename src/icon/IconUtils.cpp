/*
 * Copyright 2006-2011, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 *		Ingo Weinhold <bonefish@cs.tu-berlin.de>
 * 		John Scipione <jscipione@gmail.com>
 */


#include "IconUtils.h"

#include <new>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <TypeConstants.h>

#include "AutoDeleter.h"
#include "Icon.h"
#include "IconRenderer.h"
#include "FlatIconImporter.h"
#include "MessageImporter.h"

#ifndef HAIKU_TARGET_PLATFORM_HAIKU
#	define B_MINI_ICON_TYPE		'MICN'
#	define B_LARGE_ICON_TYPE	'ICON'
#endif

_USING_ICON_NAMESPACE;
using std::nothrow;


status_t
BIconUtils::GetVectorIcon(const uint8* buffer, size_t size, BBitmap* result)
{
	if (!result)
		return B_BAD_VALUE;

	status_t ret = result->InitCheck();
	if (ret < B_OK)
		return ret;

	BBitmap* temp = result;
	ObjectDeleter<BBitmap> deleter;

	if (result->ColorSpace() != B_RGBA32 && result->ColorSpace() != B_RGB32) {
		temp = new (nothrow) BBitmap(result->Bounds(),
			B_BITMAP_NO_SERVER_LINK, B_RGBA32);
		deleter.SetTo(temp);
		if (!temp || temp->InitCheck() != B_OK)
			return B_NO_MEMORY;
	}

	Icon icon;
	ret = icon.InitCheck();
	if (ret < B_OK)
		return ret;

	FlatIconImporter importer;
	ret = importer.Import(&icon, const_cast<uint8*>(buffer), size);
	if (ret < B_OK) {
		// try the message based format used by Icon-O-Matic
		MessageImporter messageImporter;
		BMemoryIO memoryIO(const_cast<uint8*>(buffer), size);
		ret = messageImporter.Import(&icon, &memoryIO);
		if (ret < B_OK)
			return ret;
	}

	IconRenderer renderer(temp);
	renderer.SetIcon(&icon);
	renderer.SetScale((temp->Bounds().Width() + 1.0) / 64.0);
	renderer.Render();

	if (temp != result) {
//		uint8* src = (uint8*)temp->Bits();
//		uint32 width = temp->Bounds().IntegerWidth() + 1;
//		uint32 height = temp->Bounds().IntegerHeight() + 1;
//		uint32 srcBPR = temp->BytesPerRow();
//		ret = ConvertToCMAP8(src, width, height, srcBPR, result);
ret = B_ERROR;
	}

	// TODO: would be nice to get rid of this
	// (B_RGBA32_PREMULTIPLIED or better yet, new blending_mode)
	// NOTE: probably not necessary only because
	// transparent colors are "black" in all existing icons
	// lighter transparent colors should be too dark if
	// app_server uses correct blending
//	renderer.Demultiply();

	return ret;
}
