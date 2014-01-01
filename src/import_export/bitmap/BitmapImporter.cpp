/*
 * Copyright 2014, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "BitmapImporter.h"

#include <Bitmap.h>
#include <BitmapStream.h>
#include <TranslatorRoster.h>

#include "AutoDeleter.h"
#include "Document.h"
#include "Image.h"
#include "Layer.h"
#include "RenderBuffer.h"

// constructor
BitmapImporter::BitmapImporter(const DocumentRef& document)
	: fDocument(document)
	, fTranslationFormat(0)
{
}

// destructor
BitmapImporter::~BitmapImporter()
{
}

// Import
status_t
BitmapImporter::Import(BPositionIO& stream)
{
	fTranslationFormat = 0;

	if (fDocument.Get() == NULL)
		return B_NO_INIT;

	BTranslatorRoster* roster = BTranslatorRoster::Default();
	if (roster == NULL)
		return B_ERROR;

	translator_info info;
	status_t ret = roster->Identify(&stream, NULL, &info, 0, NULL,
		B_TRANSLATOR_BITMAP);
	if (ret != B_OK)
		return ret;

	fTranslationFormat = info.type;

	BBitmapStream bitmapStream;
	ret = roster->Translate(&stream, &info, NULL, &bitmapStream,
		B_TRANSLATOR_BITMAP);
	if (ret != B_OK)
		return ret;
	
	BBitmap* bitmap = NULL;
	ret = bitmapStream.DetachBitmap(&bitmap);
	if (ret != B_OK)
		return ret;

	if (bitmap == NULL)
		return B_ERROR;
	ObjectDeleter<BBitmap> bitmapDeleter(bitmap);

	fDocument->SetBounds(bitmap->Bounds().OffsetToCopy(B_ORIGIN));

	RenderBufferRef buffer(new(std::nothrow) RenderBuffer(bitmap), true);
	if (buffer.Get() == NULL)
		return B_NO_MEMORY;

	Reference<Image> image(new(std::nothrow) Image(buffer.Get()), true);
	if (image.Get() == NULL)
		return B_NO_MEMORY;
	
	if (!fDocument->RootLayer()->AddObject(image.Get()))
		return B_NO_MEMORY;

	return B_OK;
}
