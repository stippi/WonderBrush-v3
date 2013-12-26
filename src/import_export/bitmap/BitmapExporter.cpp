/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "BitmapExporter.h"

#include <Bitmap.h>
#include <BitmapStream.h>
#include <TranslatorFormats.h>
#include <TranslatorRoster.h>

#include "Document.h"
#include "RenderManager.h"

// constructor
BitmapExporter::BitmapExporter(const BRect& bounds)
	:
	Exporter(),
	fFormat(B_PNG_FORMAT),
	fWidth(bounds.IntegerWidth() + 1),
	fHeight(bounds.IntegerHeight() + 1)
{
}

// constructor
BitmapExporter::BitmapExporter(uint32 width, uint32 height)
	:
	Exporter(),
	fFormat(B_PNG_FORMAT),
	fWidth(width),
	fHeight(height)
{
}

// destructor
BitmapExporter::~BitmapExporter()
{
}

// Export
status_t
BitmapExporter::Export(const DocumentRef& document, BPositionIO* stream)
{
	if (fWidth == 0 || fHeight == 0)
		return B_NO_INIT;

	// Attach RenderManager to document
	RenderManager renderer(document.Get());

	status_t ret = renderer.Init();
	if (ret != B_OK)
		return ret;
	
	// Trigger render
	renderer.AreaInvalidated(document->RootLayer(), document->Bounds());
	renderer.AllAreasInvalidated();

	// Wait for it to finish
	while (!renderer.RenderingDone()) {
		printf("Waiting for rendering to finish.\n");
		snooze(250000);
	}

	if (!renderer.LockDisplay())
		return B_ERROR;

	// save bitmap to translator
	BTranslatorRoster* roster = BTranslatorRoster::Default();
	if (!roster)
		return B_ERROR;

	BBitmapStream bitmapStream(const_cast<BBitmap*>(renderer.DisplayBitmap()));
	ret = roster->Translate(&bitmapStream, NULL, NULL, stream, fFormat, 0);

	BBitmap* dummy;
	bitmapStream.DetachBitmap(&dummy);

	renderer.UnlockDisplay();

	return ret;
}

// MIMEType
const char*
BitmapExporter::MIMEType()
{
	// TODO: ...
	return "image/png";
}

