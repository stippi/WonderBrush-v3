#include "ObjectTreeView.h"

#include <stdio.h>

#include <Bitmap.h>

#include "Column.h"
#include "Object.h"
#include "TextViewPopup.h"


ObjectColumnTreeItem::ObjectColumnTreeItem(float height, Object* object)
	: EasyColumnTreeItem(height)
	, object(object)
{
}


ObjectColumnTreeItem::~ObjectColumnTreeItem()
{
}


void
ObjectColumnTreeItem::Update()
{
	BBitmap icon(BRect(0, 0, 15, 15), 0, B_RGBA32);
	if (object->GetIcon(&icon))
		SetContent(1, &icon);
	SetContent(0, object->Name());
}


// #pragma mark - ObjectTreeView


enum {
	MSG_RENAME_OBJECT	= 'rnoj'
};


ObjectTreeView::ObjectTreeView(BRect frame)
	: ColumnTreeView(frame)
{
}


ObjectTreeView::~ObjectTreeView()
{
}


void
ObjectTreeView::MouseDown(BPoint where)
{
	MakeFocus(true);
	ColumnTreeView::MouseDown(where);
}


void
ObjectTreeView::KeyDown(const char* bytes, int32 numBytes)
{
	switch (bytes[0]) {
	// TODO: Some re-configurable global short cut handling...
	case 'e':
	case 'E':
	case B_F2_KEY:
		_HandleRenameSelectedItem();
		break;

	default:
		ColumnTreeView::KeyDown(bytes, numBytes);
		break;
	}
}


void
ObjectTreeView::MessageReceived(BMessage* message)
{
	switch (message->what) {
	case MSG_RENAME_OBJECT:
		_HandleRenameObject(message);
		break;
	default:
		ColumnTreeView::MessageReceived(message);
	}
}


void
ObjectTreeView::_HandleRenameSelectedItem()
{
	_HandleRenameItem(CurrentSelection(0));
}


void
ObjectTreeView::_HandleRenameItem(int32 index)
{
	ObjectColumnTreeItem* item = dynamic_cast<ObjectColumnTreeItem*>(
		ItemAt(index));

	if (item == NULL || item->object == NULL)
		return;

	item->object->AddReference();

	BMessage* message = NULL;
	try {
		message = new BMessage(MSG_RENAME_OBJECT);
		message->AddPointer("object", item->object);
		message->AddPointer("item", item);
		message->AddInt32("index", index);
	
		BRect frame(ItemFrame(index));
		frame.left += IndentationOf(item) + 9.0;
		ConvertToScreen(&frame);

		// Hide the current name in order not to irritate during editing.
		item->SetContent(0, "");

		new TextViewPopup(frame, item->object->Name(), message, this);
	} catch (...) {
		delete message;
		item->object->RemoveReference();
	}
}


void
ObjectTreeView::_HandleRenameObject(BMessage* message)
{
	Object* object;
	if (message->FindPointer("object", (void**)&object) != B_OK)
		return;

	ObjectColumnTreeItem* item;
	if (message->FindPointer("item", (void**)&item) != B_OK)
		return;

	const char* name;
	if (message->FindString("text", &name) != B_OK) {
		// Restore name which is still valid, but was hidden
		// in order not to irritate during editing.
		if (HasItem(item)) {
			item->SetContent(0, object->Name());
			InvalidateItem(item);
		}
		object->RemoveReference();
		return;
	}

	// TODO: Action!
	// TODO: Locking!
	if (strcmp(object->Name(), name) == 0) {
		// Happens when we use the TAB key to navigate without renaming
		// In this case, the item needs to be invalidated too, because
		// the name was hidden.
		if (HasItem(item)) {
			item->SetContent(0, object->Name());
			InvalidateItem(item);
		}
	} else {
		object->SetName(name);
	}
	object->RemoveReference();

	// Edit the name of the next item too?
	int32 next;
	int32 index;
	if (message->FindInt32("next", &next) != B_OK
		|| message->FindInt32("index", &index) != B_OK)
		return;

	if (index + next >= CountItems())
		return;

	_HandleRenameItem(index + next);
}

