/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "InspectorView.h"

#include <stdio.h>

#include "EditManager.h"
#include "Messenger.h"
#include "Property.h"
#include "PropertyObject.h"
#include "SetPropertiesEdit.h"

enum {
	MSG_OBJECT_CHANGED	= 'objc'
};

// constructor
InspectorView::InspectorView(EditContext& editContext)
	:
	PropertyListView(),
	fSelection(NULL),
	fEditManager(NULL),
	fEditContext(editContext),
	fObject(NULL),
	fIgnoreObjectChange(false)
{
}

// destructor
InspectorView::~InspectorView()
{
	SetMenu(NULL);
	SetSelection(NULL);
	_SetObject(NULL);
}

// MessageReceived
void
InspectorView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_OBJECT_CHANGED:
		{
			BaseObject* object;
			if (message->FindPointer("object", (void**)&object) == B_OK) {
				if (object == fObject)
					SetTo(fObject->MakePropertyObject());
				// Remove the extra reference that was transferred with
				// the message
				object->RemoveReference();
			}
			break;
		}

		default:
			PropertyListView::MessageReceived(message);
			break;
	}
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
	if (fEditManager == NULL || fObject == NULL)
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

	UndoableEdit* edit = new(std::nothrow) SetPropertiesEdit(objects, 1,
		oldObject, newObject);

	fIgnoreObjectChange = true;
	fEditManager->Perform(edit, fEditContext);
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
		BMessenger messenger(this);
		BMessage message(MSG_OBJECT_CHANGED);
		if (message.AddPointer("object", object) == B_OK
			&& messenger.SendMessage(&message) == B_OK) {
			fObject->AddReference();
		}
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

// SetEditManager
void
InspectorView::SetEditManager(EditManager* stack)
{
	fEditManager = stack;
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

