/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "InspectorView.h"

#include "CommandStack.h"
#include "Property.h"
#include "PropertyObject.h"
#include "SetPropertiesCommand.h"


// constructor
InspectorView::InspectorView()
	:
	PropertyListView(),
	fSelection(NULL),
	fCommandStack(NULL),
	fObject(NULL),
	fIgnoreObjectChange(false)
{
}

// destructor
InspectorView::~InspectorView()
{
	SetSelection(NULL);
	_SetObject(NULL);
}

// Draw
void
InspectorView::Draw(BRect updateRect)
{
	PropertyListView::Draw(updateRect);

	if (fObject != NULL)
		return;

	// display helpful messages
	const char* message1 = "Click on an object in";
	const char* message2 = "any of the other lists to";
	const char* message3 = "edit it's properties here.";

	SetHighColor(tint_color(LowColor(), B_DARKEN_2_TINT));
	font_height fh;
	GetFontHeight(&fh);
	BRect b(Bounds());

	BPoint middle;
	float textHeight = (fh.ascent + fh.descent) * 1.5;
	middle.y = (b.top + b.bottom) / 2.0 - textHeight;
	middle.x = (b.left + b.right - StringWidth(message1)) / 2.0;
	DrawString(message1, middle);

	middle.y += textHeight;
	middle.x = (b.left + b.right - StringWidth(message2)) / 2.0;
	DrawString(message2, middle);

	middle.y += textHeight;
	middle.x = (b.left + b.right - StringWidth(message3)) / 2.0;
	DrawString(message3, middle);
}

// #pragma mark -

// PropertyChanged
void
InspectorView::PropertyChanged(const Property* previous,
	const Property* current)
{
	if (fCommandStack == NULL || fObject == NULL)
		return;

	PropertyObject* oldObject = new (std::nothrow) PropertyObject();
	if (oldObject)
		oldObject->AddProperty(previous->Clone());

	PropertyObject* newObject = new (std::nothrow) PropertyObject();
	if (newObject)
		newObject->AddProperty(current->Clone());

	BaseObject** objects = new (std::nothrow) BaseObject*[1];
	if (objects)
		objects[0] = fObject;

	Command* command = new (std::nothrow) SetPropertiesCommand(objects, 1,
		oldObject, newObject);

	fIgnoreObjectChange = true;
	fCommandStack->Perform(command);
	fIgnoreObjectChange = false;
}

// PasteProperties
void
InspectorView::PasteProperties(const PropertyObject* object)
{
	// TODO: command for this
	if (fObject)
		fObject->SetToPropertyObject(object);

	PropertyListView::PasteProperties(object);
}

// IsEditingMultipleObjects
bool
InspectorView::IsEditingMultipleObjects()
{
	return PropertyListView::IsEditingMultipleObjects();
}

// #pragma mark -

// ObjectSelected
void
InspectorView::ObjectSelected(const Selectable& selected,
	const Selection::Controller* controller)
{
	_SetObject(selected.Get());
}

// ObjectDeselected
void
InspectorView::ObjectDeselected(const Selectable& selected,
	const Selection::Controller* controller)
{
	if (selected.Get() == fObject)
		_SetObject(NULL);
}

// #pragma mark -

// ObjectChanged
void
InspectorView::ObjectChanged(const Notifier* object)
{
	if (object == fObject/* && !fIgnoreObjectChange*/) {
//printf("IconObjectListView::ObjectChanged(fObject)\n");
		SetTo(fObject->MakePropertyObject());
	}
}

// #pragma mark -

// SetSelection
void
InspectorView::SetSelection(Selection* selection)
{
	if (fSelection == selection)
		return;

	if (fSelection)
		fSelection->RemoveListener(this);

	fSelection = selection;

	if (fSelection)
		fSelection->AddListener(this);
}

// SetCommandStack
void
InspectorView::SetCommandStack(CommandStack* stack)
{
	fCommandStack = stack;
}

// #pragma mark -

// _SetObject
void
InspectorView::_SetObject(BaseObject* object)
{
	if (fObject == object)
		return;

	if (fObject) {
		fObject->RemoveListener(this);
		fObject->RemoveReference();
	}

	fObject = object;
	PropertyObject* propertyObject = NULL;

	if (fObject) {
		fObject->AddReference();
		fObject->AddListener(this);
		propertyObject = fObject->MakePropertyObject();
	}

	SetTo(propertyObject);
}

