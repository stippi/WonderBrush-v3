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
BitmapExporter::BitmapExporter()
	:
	Exporter(),
	fWidth(0),
	fHeight(0)
{
	SetFormat(B_PNG_FORMAT);
}

// constructor
BitmapExporter::BitmapExporter(const BRect& bounds)
	:
	Exporter(),
	fWidth(bounds.IntegerWidth() + 1),
	fHeight(bounds.IntegerHeight() + 1)
{
	SetFormat(B_PNG_FORMAT);
}

// constructor
BitmapExporter::BitmapExporter(uint32 width, uint32 height)
	:
	Exporter(),
	fWidth(width),
	fHeight(height)
{
	SetFormat(B_PNG_FORMAT);
}

// destructor
BitmapExporter::~BitmapExporter()
{
}

// Export
status_t
BitmapExporter::Export(const DocumentRef& document, BPositionIO* stream)
{
	// Attach RenderManager to document
	RenderManager renderer(document.Get());

	// TODO: Use fWidth and fHeight if non-zero!

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
	if (roster == NULL)
		return B_ERROR;

	translator_info info;
	info.type = fFormat.type;
	info.translator = fTranslatorID;
	info.group = fFormat.group;
	info.quality = fFormat.quality;
	info.capability = fFormat.capability;
	memcpy(info.name, fFormat.name, sizeof(info.name));
	memcpy(info.MIME, fFormat.MIME, sizeof(info.name));

	BBitmapStream bitmapStream(const_cast<BBitmap*>(renderer.DisplayBitmap()));
	ret = roster->Translate(&bitmapStream, &info, NULL, stream, fFormat.type,
		0);

	BBitmap* dummy;
	bitmapStream.DetachBitmap(&dummy);

	renderer.UnlockDisplay();

	return ret;
}

// MIMEType
const char*
BitmapExporter::MIMEType()
{
	return fFormat.MIME;
}

// SetFormat
void
BitmapExporter::SetFormat(uint32 format)
{
	BTranslatorRoster* roster = BTranslatorRoster::Default();
	if (roster == NULL)
		return;

	translator_id* translatorIDs;
	int32 translatorCount;
	if (roster->GetAllTranslators(&translatorIDs, &translatorCount) != B_OK)
		return;

	float bestQuality = 0.0f;
	float bestCapability = 0.0f;

	for (int32 i = 0; i < translatorCount; i++) {
		const translation_format* formats;
		int32 formatCount;
		if (roster->GetOutputFormats(translatorIDs[i], &formats,
				&formatCount) != B_OK) {
			break;
		}
		
		for (int32 j = 0; j < formatCount; j++) {
			if (formats[j].type != format)
				continue;
			if (formats[j].quality > bestQuality
				&& formats[j].capability > bestCapability) {
				fFormat = formats[j];
				fTranslatorID = translatorIDs[i];
				bestQuality = formats[j].quality;
				bestCapability = formats[j].quality;
				printf("found Translator: %ld, %.1f, %.1f\n", fTranslatorID,
					bestQuality, bestCapability);
			}
		}
	}

	delete[] translatorIDs;
}

