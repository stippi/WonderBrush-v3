/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "ResourceTreeView.h"

#include <new>
#include <stdio.h>

#include <Bitmap.h>
#include <Window.h>

#include "AutoDeleter.h"
#include "Column.h"
#include "ColumnTreeViewColors.h"
#include "EditManager.h"
#include "RenameObjectEdit.h"
#include "TextViewPopup.h"

using std::nothrow;


ResourceColumnTreeItem::ResourceColumnTreeItem(float height,
		BaseObject* object)
	: EasyColumnTreeItem(height)
	, object(object)
{
	// No reference to the object is added, since we only need
	// the object to be valid when Update is called, which the
	// parent list view makes sure of.
}


ResourceColumnTreeItem::~ResourceColumnTreeItem()
{
}


void
ResourceColumnTreeItem::Update()
{
//	BBitmap icon(BRect(0, 0, 15, 15), 0, B_RGBA32);
//	if (object->GetIcon(&icon))
//		SetContent(1, &icon);
	SetContent(0, object->Name());
}


// #pragma mark - ResourceTreeView


enum {
	MSG_RENAME_OBJECT			= 'rnoj',
	MSG_RENAME_SELECTED_ITEM	= 'rnit',
	MSG_OBJECT_SELECTED			= 'objs'
};


// constructor
ResourceTreeView::ResourceTreeView(BRect frame, Document* document,
		Selection* selection)
	:
	ColumnTreeView(frame),
	fDocument(document),
	fSelection(selection),
	fResourceObserver(this),
	fIgnoreSelectionChanged(false)
{
	_Init();
}

#ifdef __HAIKU__

// constructor
ResourceTreeView::ResourceTreeView(Document* document, Selection* selection)
	:
	ColumnTreeView(),
	fDocument(document),
	fSelection(selection),
	fResourceObserver(this),
	fIgnoreSelectionChanged(false)
{
	_Init();
}

#endif // __HAIKU__

// destructor
ResourceTreeView::~ResourceTreeView()
{
}

// AttachedToWindow
void
ResourceTreeView::AttachedToWindow()
{
	ColumnTreeView::AttachedToWindow();
	Window()->AddShortcut('e', B_COMMAND_KEY,
		new BMessage(MSG_RENAME_SELECTED_ITEM), this);

	fSelection->AddListener(this);

	if (fDocument->WriteLock()) {
		_AddItems(fDocument->GlobalResources(), NULL);
		fDocument->WriteUnlock();
	}
}

// DetachedFromWindow
void
ResourceTreeView::DetachedFromWindow()
{
	fSelection->RemoveListener(this);
	Window()->RemoveShortcut('e', B_COMMAND_KEY);
	ColumnTreeView::DetachedFromWindow();
}

// MouseDown
void
ResourceTreeView::MouseDown(BPoint where)
{
	MakeFocus(true);
	ColumnTreeView::MouseDown(where);
}

// KeyDown
void
ResourceTreeView::KeyDown(const char* bytes, int32 numBytes)
{
	switch (bytes[0]) {
		// TODO: Some re-configurable global short cut handling...
		case 'e':
		case 'E':
			_HandleRenameSelectedItem();
			break;
		case B_FUNCTION_KEY:
			if (BMessage* message = Window()->CurrentMessage()) {
				int32 key;
				if (message->FindInt32("key", &key) == B_OK) {
					switch (key) {
						case B_F2_KEY:
							_HandleRenameSelectedItem();
							break;
						default:
							break;
					}
				}
			}
			break;

		default:
			ColumnTreeView::KeyDown(bytes, numBytes);
			break;
	}
}

// MessageReceived
void
ResourceTreeView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_RENAME_SELECTED_ITEM:
			_HandleRenameSelectedItem();
			break;
		case MSG_RENAME_OBJECT:
			_HandleRenameObject(message);
			break;

		case ResourceList::Observer::MSG_OBJECT_ADDED:
		case ResourceList::Observer::MSG_OBJECT_REMOVED:
//		case ResourceList::Observer::MSG_OBJECT_CHANGED:
			if (fDocument->WriteLock()) {
				const ResourceList* list;
				BaseObject* object;
				int32 index;
				if (message->FindPointer("list", (void**)&list) == B_OK
					&& message->FindPointer("object", (void**)&object) == B_OK
					&& message->FindInt32("index", &index) == B_OK) {
					if (message->what
						== ResourceList::Observer::MSG_OBJECT_ADDED) {
						_ObjectAdded(list, object, index);
					} else if (message->what
						== ResourceList::Observer::MSG_OBJECT_REMOVED) {
						_ObjectRemoved(list, object, index);
//					} else if (message->what
//						== ResourceList::Observer::MSG_OBJECT_CHANGED) {
//						_ObjectChanged(list, object, index);
					}
				}
				fDocument->WriteUnlock();
			}
			break;

		case MSG_OBJECT_SELECTED:
		{
			BaseObject* object;
			bool selected;
			if (message->FindPointer("object", (void**)&object) == B_OK
				&& message->FindBool("selected", &selected) == B_OK) {
				_ObjectSelected(object, selected);
			}
			break;
		}

		default:
			ColumnTreeView::MessageReceived(message);
	}
}

// SelectionChanged
void
ResourceTreeView::SelectionChanged()
{
	// Do not mess with the selection while adding/removing items.
	if (fIgnoreSelectionChanged)
		return;

	// Sync the selections
	int32 count = CountSelectedItems();
	if (count == 0) {
		fSelection->DeselectAll(this);
		return;
	}
	bool extend = false;
	for (int32 i = 0; i < count; i++) {
		ResourceColumnTreeItem* item = dynamic_cast<ResourceColumnTreeItem*>(
			ItemAt(CurrentSelection(i)));
		if (item == NULL)
			continue;
		fSelection->Select(Selectable(item->object), this, extend);
		extend = true;
	}
}

// #pragma mark -

// ObjectSelected
void
ResourceTreeView::ObjectSelected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	BaseObject* object = selectable.Get();
	if (object == NULL || Window() == NULL)
		return;

	// Theoretically, all selection notifications are triggered in the
	// correct thread, i.e. the window thread, since the selection is
	// unique per window. But the object added/removed notifications can
	// be triggered from other window threads and need to be asynchronous.
	// This means the selection notifications need to be asynchronous as
	// well, to keep the chain of events in order.

	BMessage message(MSG_OBJECT_SELECTED);
	message.AddPointer("object", object);
	message.AddBool("selected", true);
	Window()->PostMessage(&message, this);
}

// ObjectDeselected
void
ResourceTreeView::ObjectDeselected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	BaseObject* object = selectable.Get();
	if (object == NULL || Window() == NULL)
		return;

	// See ObjectSelected() on why we need an asynchronous notification.

	BMessage message(MSG_OBJECT_SELECTED);
	message.AddPointer("object", object);
	message.AddBool("selected", false);
	Window()->PostMessage(&message, this);
}

// #pragma mark -

// _Init
void
ResourceTreeView::_Init()
{
	AddColumn(new Column("Name", "name", 177,
		COLUMN_MOVABLE | COLUMN_VISIBLE));

	AddColumn(new Column("", "icon", 18,
		COLUMN_MOVABLE | COLUMN_VISIBLE));
}

// _HandleRenameSelectedItem
void
ResourceTreeView::_HandleRenameSelectedItem()
{
	_HandleRenameItem(CurrentSelection(0));
}

// _HandleRenameItem
void
ResourceTreeView::_HandleRenameItem(int32 index)
{
	ResourceColumnTreeItem* item = dynamic_cast<ResourceColumnTreeItem*>(
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

		AutoReadLocker locker(fDocument);

		new TextViewPopup(frame, item->object->Name(), message, this);
	} catch (...) {
		delete message;
		item->object->RemoveReference();
	}
}

// _HandleRenameObject
void
ResourceTreeView::_HandleRenameObject(BMessage* message)
{
	BaseObject* object;
	if (message->FindPointer("object", (void**)&object) != B_OK)
		return;

	ResourceColumnTreeItem* item;
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

	AutoWriteLocker locker(fDocument);

	if (strcmp(object->Name(), name) == 0) {
		// Happens when we use the TAB key to navigate without renaming
		// In this case, the item needs to be invalidated too, because
		// the name was hidden.
		if (HasItem(item)) {
			item->SetContent(0, object->Name());
			InvalidateItem(item);
		}
	} else {
		// rename via edit
		RenameObjectEdit* edit = new (nothrow) RenameObjectEdit(object,
			name);
		fDocument->EditManager()->Perform(edit);
	}
	object->RemoveReference();

	locker.Unlock();

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

// _ObjectAdded
void
ResourceTreeView::_ObjectAdded(const ResourceList* list, BaseObject* object,
	int32 index)
{
//printf("ResourceTreeView::_ObjectAdded(%p, %p, %ld)\n", layer, object, index);
	if (!list->HasObject(object))
		return;

	ResourceColumnTreeItem* item
		= new(std::nothrow) ResourceColumnTreeItem(20, object);
	if (item == NULL)
		return;
	item->Update();

	fIgnoreSelectionChanged = true;

	AddItem(item, index);
	// Check successful insertion and expand item
	if (SubItemAt(NULL, index) == item)
		ExpandItem(item);
	else
		delete item;

	fIgnoreSelectionChanged = false;
}

// _ObjectRemoved
void
ResourceTreeView::_ObjectRemoved(const ResourceList* list, BaseObject* object,
	int32 index)
{
	fIgnoreSelectionChanged = true;

	// TODO: A bug in DefaultColumnTreeModel already deleted the returned
	// item:
	RemoveItem(index);

	fIgnoreSelectionChanged = false;
}

// _ObjectChanged
void
ResourceTreeView::_ObjectChanged(const ResourceList* list, BaseObject* object,
	int32 index)
{
	ResourceColumnTreeItem* item = _FindLayerTreeViewItem(object);
	if (!item)
		return;
	item->Update();
	InvalidateItem(item);
}

// _ObjectSelected
void
ResourceTreeView::_ObjectSelected(BaseObject* object, bool selected)
{
	fIgnoreSelectionChanged = true;

	ColumnTreeItem* item = _FindLayerTreeViewItem(object);
	if (item != NULL) {
		int32 index = IndexOf(item);
		if (selected) {
//printf("selecting: %ld\n", index);
			Select(index, true);
		} else {
//printf("deselecting: %ld\n", index);
			Deselect(index);
		}
	}

	ScrollToSelection();

	fIgnoreSelectionChanged = false;
}

// _FindLayerTreeViewItem
ResourceColumnTreeItem*
ResourceTreeView::_FindLayerTreeViewItem(const BaseObject* object)
{
	int32 count = CountItems();
	for (int32 i = 0; i < count; i++) {
		ResourceColumnTreeItem* item = dynamic_cast<ResourceColumnTreeItem*>(
			ItemAt(i));
		if (item && item->object == object)
			return item;
	}
	return NULL;
}

// _AddItems
void
ResourceTreeView::_AddItems(const ResourceList& list,
	ResourceColumnTreeItem* parentItem)
{
	int32 count = list.CountObjects();
	for (int32 i = 0; i < count; i++) {
		BaseObject* object = list.ObjectAtFast(i);

		ResourceColumnTreeItem* item = new ResourceColumnTreeItem(20, object);
		item->Update();

		if (parentItem)
			AddSubItem(parentItem, item, i);
		else
			AddItem(item, i);
		ExpandItem(item);
	}
}
