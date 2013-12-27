/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "MessageExporter.h"

#include <ByteOrder.h>
#include <DataIO.h>
#include <Message.h>
#include <TypeConstants.h>

#include "DocumentVisitor.h"

// constructor
MessageExporter::MessageExporter()
{
}

// destructor
MessageExporter::~MessageExporter()
{
}

static const char* kType = "type";

class ArchiveVisitor : public DocumentVisitor<BMessage> {
	typedef DocumentVisitor<BMessage> inherited;
	
public:
	ArchiveVisitor()
		: status(B_OK)
	{
	}	
	
	virtual bool VisitDocument(Document* document, BMessage* context)
	{
		return inherited::VisitDocument(document, context);
	}

	virtual bool VisitLayer(Layer* layer, BMessage* context)
	{
		BMessage archive;
		if (!inherited::VisitLayer(layer, &archive))
			return false;

		return context->AddMessage("layer", &archive) == B_OK;
	}

	virtual bool VisitObject(Object* object, BMessage* context)
	{
		BMessage archive;
		if (!inherited::VisitObject(object, &archive))
			return false;
		
		status = B_OK;

		const BString& name = object->GivenName();
		if (status == B_OK && name.Length() > 0)
			status = archive.AddString("name", name);

		if (status == B_OK)
			status = context->AddMessage("object", &archive);
		
		return status == B_OK;
	}

	virtual bool VisitFilter(Filter* filter, BMessage* context)
	{
		status = context->AddString(kType, "FilterGaussianBlur");
		return status == B_OK;
	}

	virtual bool VisitFilterDropShadow(FilterDropShadow* dropShadow,
		BMessage* context)
	{
		status = context->AddString(kType, "FilterDropShadow");
		return status == B_OK;
	}

	virtual bool VisitFilterSaturation(FilterSaturation* saturation,
		BMessage* context)
	{
		status = context->AddString(kType, "FilterSaturation");
		return status == B_OK;
	}

	virtual bool VisitBrushStroke(BrushStroke* stroke, BMessage* context)
	{
		status = context->AddString(kType, "BrushStroke");
		return status == B_OK;
	}

	virtual bool VisitImage(Image* image, BMessage* context)
	{
		status = context->AddString(kType, "Image");
		return status == B_OK;
	}

	virtual bool VisitRect(Rect* rect, BMessage* context)
	{
		status = context->AddString(kType, "Rect");
		return status == B_OK;
	}

	virtual bool VisitShape(Shape* shape, BMessage* context)
	{
		status = context->AddString(kType, "Shape");
		return status == B_OK;
	}

	virtual bool VisitText(Text* text, BMessage* context)
	{
		status = context->AddString(kType, "Text");
		return status == B_OK;
	}

public:
	status_t	status;
};

// Export
status_t
MessageExporter::Export(const DocumentRef& document, BPositionIO* stream)
{
	status_t ret = B_OK;

	// Prepend the magic number to the file which later tells us that this file
	// is one of us
	if (ret == B_OK) {
		ssize_t size = sizeof(uint32);
		uint32 magic = B_HOST_TO_BENDIAN_INT32('WBI2');
		ssize_t written = stream->Write(&magic, size);
		if (written != size) {
			if (written < 0)
				ret = (status_t)written;
			else
				ret = B_IO_ERROR;
		}
	}

	if (ret == B_OK) {
		BMessage archive;
		
		ArchiveVisitor visitor;
		visitor.VisitDocument(document.Get(), &archive);
		ret = visitor.status;

		archive.PrintToStream();

		if (ret == B_OK)
			ret = archive.Flatten(stream);
	}

	return ret;
}

// MIMEType
const char*
MessageExporter::MIMEType()
{
	return "image/x-wonderbrush-2";
}




