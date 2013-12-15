/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "Document.h"

#include <new>

#include "DocumentVisitor.h"
#include "EditManager.h"
#include "Layer.h"
#include "Object.h"
#include "Path.h"
#include "Style.h"


// constructor
Document::Listener::Listener()
{
}

// destructor
Document::Listener::~Listener()
{
}

// #pragma mark -

static const char* kLockName = "document rw lock";

// constructor
Document::Document(const BRect& bounds)
	:
	RWLocker(kLockName),
	fEditManager(new(std::nothrow) ::EditManager(this)),
	fRootLayer(new(std::nothrow) Layer(bounds)),
	fGlobalResources(),
	fListeners(8)
{
}

// destructor
Document::~Document()
{
	delete fEditManager;
	delete fRootLayer;
}

// #pragma mark -

// DefaultName
const char*
Document::DefaultName() const
{
	return "Unnamed";
}

// #pragma mark -

// InitCheck
status_t
Document::InitCheck() const
{
	return fEditManager && fRootLayer ? B_OK : B_NO_MEMORY;
}

// Bounds
BRect
Document::Bounds() const
{
	return fRootLayer->Bounds();
}

// #pragma mark -

// AddListener
bool
Document::AddListener(Listener* listener)
{
	if (listener == NULL || fListeners.HasItem(listener))
		return false;
	return fListeners.AddItem(listener);
}

// RemoveListener
void
Document::RemoveListener(Listener* listener)
{
	fListeners.RemoveItem(listener);
}

// #pragma mark -

// HasLayer
bool
Document::HasLayer(Layer* layer) const
{
	if (!layer)
		return false;
	return _HasLayer(fRootLayer, layer);
}

class Indentation {
public:
	Indentation()
		: level(0)
	{
	}

	void Increase()
	{
		level++;
	}

	void Decrease()
	{
		level--;
	}

	int32	level;
};

class PrintVisitor : public DocumentVisitor<Indentation> {
public:
	typedef DocumentVisitor<Indentation> inherited;
	
	PrintVisitor(Document* document)
	{
		Indentation indentation;
		VisitDocument(document, &indentation);
	}

	virtual bool VisitLayer(Layer* layer, Indentation* context)
	{
		_PrintIndented("Layer {", context);
		context->Increase();

		if (!inherited::VisitLayer(layer, context))
			return false;

		context->Decrease();
		_PrintIndented("}", context);
		return true;
	}

	virtual bool VisitFilter(Filter* filter, Indentation* context)
	{
		_PrintIndented("Gaussian", context);
		return true;
	}

	virtual bool VisitFilterDropShadow(FilterDropShadow* dropShadow,
		Indentation* context)
	{
		_PrintIndented("Drop shadow", context);
		return true;
	}

	virtual bool VisitFilterSaturation(FilterSaturation* saturation,
		Indentation* context)
	{
		_PrintIndented("Saturation", context);
		return true;
	}

	virtual bool VisitBrushStroke(BrushStroke* stroke, Indentation* context)
	{
		_PrintIndented("BrushStroke", context);
		return true;
	}

	virtual bool VisitImage(Image* image, Indentation* context)
	{
		_PrintIndented("Image", context);
		return true;
	}

	virtual bool VisitRect(Rect* rect, Indentation* context)
	{
		_PrintIndented("Rect", context);
		return true;
	}

	virtual bool VisitShape(Shape* shape, Indentation* context)
	{
		_PrintIndented("Shape", context);
		return true;
	}

	virtual bool VisitText(Text* text, Indentation* context)
	{
		_PrintIndented("Text", context);
		return true;
	}

private:
	void _PrintIndented(const char* string, Indentation* context)
	{
		for (int32 i = 0; i < context->level; i++)
			printf(" ");
		printf("%s\n", string);
	}
};

// PrintToStream
void
Document::PrintToStream()
{
	PrintVisitor visitor(this);
}

// #pragma mark -

// _HasLayer
bool
Document::_HasLayer(Layer* parent, Layer* child) const
{
	if (parent == child)
		return true;

	int32 count = parent->CountObjects();
	for (int32 i = 0; i < count; i++) {
		Object* object = parent->ObjectAtFast(i);
		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer && _HasLayer(subLayer, child))
			return true;
	}

	return false;
}


