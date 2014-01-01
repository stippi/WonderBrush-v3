/*
 * Copyright 2014, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "BitmapImporter.h"

#include <Bitmap.h>
#include <TranslationUtils.h>

#include "AutoDeleter.h"
#include "Document.h"
#include "Image.h"
#include "Layer.h"
#include "RenderBuffer.h"

// constructor
BitmapImporter::BitmapImporter(const DocumentRef& document)
	: fDocument(document)
{
}

// destructor
BitmapImporter::~BitmapImporter()
{
}

// Import
status_t
BitmapImporter::Import(BPositionIO& stream) const
{
	if (fDocument.Get() == NULL)
		return B_NO_INIT;

	BBitmap* bitmap = BTranslationUtils::GetBitmap(&stream);
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
