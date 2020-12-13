/*
 * Copyright 2009-2018, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "ObjectTreeView.h"

#include <new>
#include <stdio.h>

#include <Bitmap.h>
#include <Window.h>

#include "AutoDeleter.h"
#include "Column.h"
#include "ColumnTreeViewColors.h"
#include "EditManager.h"
#include "Document.h"
#include "MoveObjectsEdit.h"
#include "MovePathsEdit.h"
#include "Object.h"
#include "PathInstance.h"
#include "RenameObjectEdit.h"
#include "TextViewPopup.h"

using std::nothrow;


ObjectColumnTreeItem::ObjectColumnTreeItem(float height, BaseObject* object)
	: EasyColumnTreeItem(height)
	, object(object)
{
	object->AddReference();
	// If "object" is of type Object, we will get notified of changes
	// via BMessages from the LayerObserver. For other types of
	// BaseObject, we need to register our own listener.
	if (dynamic_cast<Object*>(object) == NULL)
		object->AddListener(this);
}


ObjectColumnTreeItem::~ObjectColumnTreeItem()
{
	if (dynamic_cast<Object*>(object) == NULL)
		object->RemoveListener(this);
	object->RemoveReference();
}

void
ObjectColumnTreeItem::ObjectChanged(const Notifier* object)
{
	if (object == this->object)
		Update();
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
	MSG_RENAME_OBJECT			= 'rnoj',
	MSG_RENAME_SELECTED_ITEM	= 'rnit',
	MSG_DRAG_SORT_OBJECTS		= 'drgo',
	MSG_OBJECT_SELECTED			= 'objs'
};


// constructor
ObjectTreeView::ObjectTreeView(BRect frame, Document* document,
		Selection* selection, EditContext& editContext)
	:
	ColumnTreeView(frame),
	fDocument(document),
	fSelection(selection),
	fEditContext(editContext),
	fLayerObserver(this),
	fShapeObserver(this),
	fIgnoreSelectionChanged(false)
{
	_Init();
}

#ifdef __HAIKU__

// constructor
ObjectTreeView::ObjectTreeView(Document* document, Selection* selection,
		EditContext& editContext)
	:
	ColumnTreeView(),
	fDocument(document),
	fSelection(selection),
	fEditContext(editContext),
	fLayerObserver(this),
	fShapeObserver(this),
	fIgnoreSelectionChanged(false)
{
	_Init();
}

#endif // __HAIKU__

// destructor
ObjectTreeView::~ObjectTreeView()
{
}

// AttachedToWindow
void
ObjectTreeView::AttachedToWindow()
{
	ColumnTreeView::AttachedToWindow();
	Window()->AddShortcut('e', B_COMMAND_KEY,
		new BMessage(MSG_RENAME_SELECTED_ITEM), this);

	fSelection->AddListener(this);

	if (fDocument->WriteLock()) {
		_RecursiveAddItems(fDocument->RootLayer(), NULL);
		fDocument->WriteUnlock();
	}
}

// DetachedFromWindow
void
ObjectTreeView::DetachedFromWindow()
{
	fSelection->RemoveListener(this);
	Window()->RemoveShortcut('e', B_COMMAND_KEY);
	ColumnTreeView::DetachedFromWindow();
}

// MouseDown
void
ObjectTreeView::MouseDown(BPoint where)
{
	MakeFocus(true);
	ColumnTreeView::MouseDown(where);
}

// KeyDown
void
ObjectTreeView::KeyDown(const char* bytes, int32 numBytes)
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
ObjectTreeView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_RENAME_SELECTED_ITEM:
			_HandleRenameSelectedItem();
			break;
		case MSG_RENAME_OBJECT:
			_HandleRenameObject(message);
			break;

		case LayerObserver::MSG_OBJECT_ADDED:
		case LayerObserver::MSG_OBJECT_REMOVED:
		case LayerObserver::MSG_OBJECT_CHANGED:
			if (fDocument->WriteLock()) {
				Layer* layer;
				Object* object;
				int32 index;
				if (message->FindPointer("layer", (void**)&layer) == B_OK
					&& message->FindPointer("object", (void**)&object) == B_OK
					&& message->FindInt32("index", &index) == B_OK) {
					if (!fDocument->HasLayer(layer)) {
						fDocument->WriteUnlock();
						break;
					}
					if (message->what == LayerObserver::MSG_OBJECT_ADDED)
						_ObjectAdded(layer, object, index);
					else if (message->what == LayerObserver::MSG_OBJECT_REMOVED)
						_ObjectRemoved(layer, object, index);
					else
						_ObjectChanged(layer, object, index);
				}
				fDocument->WriteUnlock();
			}
			break;
		case LayerObserver::MSG_AREA_INVALIDATED:
			// not interested
			break;

		case ShapeObserver::MSG_PATH_ADDED:
		case ShapeObserver::MSG_PATH_REMOVED:
		{
			if (fDocument->WriteLock()) {
				Shape* shape;
				PathInstance* path;
				int32 index;
				if (message->FindPointer("shape", (void**)&shape) == B_OK
					&& message->FindPointer("path", (void**)&path) == B_OK
					&& message->FindInt32("index", &index) == B_OK) {
					if (message->what == ShapeObserver::MSG_PATH_ADDED)
						_PathAdded(shape, path, index);
					else if (message->what == ShapeObserver::MSG_PATH_REMOVED)
						_PathRemoved(shape, path, index);
				}
				fDocument->WriteUnlock();
			}
			break;
		}

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

		case MSG_DRAG_SORT_OBJECTS:
			// Eat this message, we have already processed it via HandleDrop().
			break;

		default:
			ColumnTreeView::MessageReceived(message);
	}
}

// InitiateDrag
bool
ObjectTreeView::InitiateDrag(BPoint point, int32 index, bool wasSelected,
	BMessage* _message)
{
	try {
		BMessage dragMessage(MSG_DRAG_SORT_OBJECTS);
		float totalHeight = -1.0f;
		float maxHeight = 100.0f;
		bool fadeOutAtBottom = false;
		int32 count = CountSelectedItems();
		int32 addedItems = 0;
		for (int32 i = 0; i < count; i++) {
			ObjectColumnTreeItem* item = dynamic_cast<ObjectColumnTreeItem*>(
				ItemAt(CurrentSelection(i)));
			if (item == NULL || item->object == NULL)
				continue;
			PathInstance* path = dynamic_cast<PathInstance*>(item->object);
			if (path != NULL) {
				if (dragMessage.AddPointer("path", path) != B_OK)
					return false;
			} else {
				if (dragMessage.AddPointer("object", item->object) != B_OK)
					return false;
			}
			addedItems++;
			if (!fadeOutAtBottom) {
				totalHeight += item->Height() + 1.0;
				fadeOutAtBottom = totalHeight > maxHeight;
			}
		}
		if (addedItems == 0)
			return false;

		BRect bitmapBounds(0, 0, Bounds().Width(), totalHeight);
		BBitmap* dragBitmap = new BBitmap(bitmapBounds, B_BITMAP_ACCEPTS_VIEWS,
			B_RGBA32);
		ObjectDeleter<BBitmap> bitmapDeleter(dragBitmap);
		BView* view = new BView(bitmapBounds, "offscreen", 0, 0);
		dragBitmap->AddChild(view);
		if (!dragBitmap->Lock())
			return false;

		// configure view to match ourself
		view->SetDrawingMode(DrawingMode());
		view->SetLowColor(LowColor());
		view->SetHighColor(HighColor());
		BFont font;
		GetFont(&font);
		view->SetFont(&font);

		float previousItemBottom = -1.0;
		for (int32 i = 0; i < count; i++) {
			ColumnTreeItem* item = ItemAt(CurrentSelection(i));
			if (item == NULL)
				continue;

			BRect itemRect(bitmapBounds);
			itemRect.top = previousItemBottom + 1.0;
			itemRect.bottom = itemRect.top + item->Height();

			for (int32 c = 0; c < CountColumns(); c++) {
				Column* column = _VisibleColumnAt(c);
				BRect columnRect = _VisibleColumnFrame(column);
				columnRect.top = itemRect.top;
				columnRect.bottom = itemRect.bottom;
				item->Draw(view, column, columnRect, columnRect,
					0, &Colors()->item_colors);
			}

			previousItemBottom = itemRect.bottom;
			if (previousItemBottom > totalHeight)
				break;
		}
		view->SetHighColor(tint_color(view->LowColor(), B_DARKEN_2_TINT));
		view->StrokeRect(view->Bounds());
		view->Sync();
		dragBitmap->Unlock();

		// Mess with the alpha channel
		uint8* row = (uint8*)dragBitmap->Bits();
		int32 bpr = dragBitmap->BytesPerRow();
		int32 width = bitmapBounds.IntegerWidth() + 1;
		int32 height = bitmapBounds.IntegerHeight() + 1;
		for (int32 y = 0; y < height; y++) {
			uint8* p = row;
			uint8 alpha = 110;
			if (fadeOutAtBottom && y >= height - 40)
				alpha = alpha * (height - y) / 40;
			for (int32 x = 0; x < width; x++) {
				p[3] = (p[3] * alpha) >> 8;
				p += 4;
			}
			row += bpr;
		}

		bitmapDeleter.Detach();

		DragMessage(&dragMessage, dragBitmap, B_OP_ALPHA, BPoint(point.x,
			point.y - ItemFrame(CurrentSelection(0)).top));

		if (_message)
			*_message = dragMessage;

		return true;

	} catch (...) {
	}
	return false;
}

// GetDropInfo
bool
ObjectTreeView::GetDropInfo(BPoint point, const BMessage& dragMessage,
	ColumnTreeItem** _super, int32* _subItemIndex, int32* _itemIndex)
{
	type_code typeFound;

	int32 pathsFound;
	if (dragMessage.GetInfo("path", &typeFound, &pathsFound) != B_OK
		|| typeFound != B_POINTER_TYPE) {
		pathsFound = 0;
	}

	int32 objectsFound;
	if (dragMessage.GetInfo("object", &typeFound, &objectsFound) != B_OK
		|| typeFound != B_POINTER_TYPE) {
		objectsFound = 0;
	}

	if (pathsFound == 0 && objectsFound == 0)
		return false;

	// Check the situation that a Layer does not have any children yet.
	// In that case we must make it possible to drop items as children,
	// but the ColumnTreeView does not know about this situation.
	int32 index = IndexOf(point);
	if (index >= 0) {
		ObjectColumnTreeItem* item = dynamic_cast<ObjectColumnTreeItem*>(
			ItemAt(index));

		int32 subItemCount = CountSubItems(item);
		if (dynamic_cast<Layer*>(item->object) != NULL
			&& objectsFound > 0
			&& (subItemCount == 0 || !item->IsExpanded())) {

			BRect frame = ItemFrame(index);
			if (point.y > frame.top + frame.Height() / 4
				&& point.y < frame.bottom - frame.Height() / 4) {
				// Drop will add items as new children.
				*_super = item;
				*_subItemIndex = subItemCount;
				*_itemIndex = index + 1;
				return true;
			}
		}
		if (dynamic_cast<Shape*>(item->object) != NULL
			&& pathsFound > 0
			&& (subItemCount == 0 || !item->IsExpanded())) {

			BRect frame = ItemFrame(index);
			if (point.y > frame.top + frame.Height() / 4
				&& point.y < frame.bottom - frame.Height() / 4) {
				// Drop will add items as new children.
				*_super = item;
				*_subItemIndex = subItemCount;
				*_itemIndex = index + 1;
				return true;
			}
		}
		
		if (dynamic_cast<PathInstance*>(item->object) != NULL) {
			if (pathsFound == 0)
				return false;
		} else {
			if (objectsFound == 0)
				return false;
		}
	}

	// TODO: We need to check if the user is hovering items that are being
	// dragged. There are situations in which this is valid -- for example
	// when making a non-contiguous selection, it shall be possible to drop
	// the items such that they are inserted at the drop index as contiguous
	// sibblings. But when dragging a layer, it should not be possible to
	// drop the layer in the hierarchy below that layer, unless a copy is
	// being made.

	return ColumnTreeView::GetDropInfo(point, dragMessage, _super,
		_subItemIndex, _itemIndex);
}

// HandleDrop
void
ObjectTreeView::HandleDrop(const BMessage& dragMessage, ColumnTreeItem* super,
	int32 subItemIndex, int32 itemIndex)
{
	Layer* insertionLayer = fDocument->RootLayer();
	Shape* insertionShape = NULL;
	if (super != NULL) {
		ObjectColumnTreeItem* item
			= dynamic_cast<ObjectColumnTreeItem*>(super);
		if (item == NULL)
			return;

		insertionLayer = dynamic_cast<Layer*>(item->object);
		insertionShape = dynamic_cast<Shape*>(item->object);
	}

	type_code type;
	int32 count;
	if (dragMessage.GetInfo("object", &type, &count) == B_OK
		&& type == B_POINTER_TYPE && insertionLayer != NULL) {
		
		_DropObjects(dragMessage, count, insertionLayer, subItemIndex,
			itemIndex);
	} else if (dragMessage.GetInfo("path", &type, &count) == B_OK
		&& type == B_POINTER_TYPE && insertionShape != NULL) {
	
		_DropPaths(dragMessage, count, insertionShape, subItemIndex,
			itemIndex);
	}
}

// SelectionChanged
void
ObjectTreeView::SelectionChanged()
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
		ObjectColumnTreeItem* item = dynamic_cast<ObjectColumnTreeItem*>(
			ItemAt(CurrentSelection(i)));
		if (item == NULL)
			continue;
		fSelection->Select(Selectable(item->object), this, extend);
		extend = true;
	}
}

// ItemSelectedTwice
void
ObjectTreeView::ItemSelectedTwice(int32 index)
{
	_HandleRenameItem(index);
}

// #pragma mark -

// ObjectSelected
void
ObjectTreeView::ObjectSelected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	BaseObject* object = dynamic_cast<BaseObject*>(selectable.Get());
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
ObjectTreeView::ObjectDeselected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	BaseObject* object = dynamic_cast<BaseObject*>(selectable.Get());
	if (object == NULL || Window() == NULL)
		return;

	// See ObjectSelected() on why we need an asynchronous notification.

	BMessage message(MSG_OBJECT_SELECTED);
	message.AddPointer("object", object);
	message.AddBool("selected", false);
	Window()->PostMessage(&message, this);
}

// #pragma mark -

// SelectItem
void
ObjectTreeView::SelectItem(BaseObject* object)
{
printf("ObjectTreeView::SelectItem(%p)\n", object);
	ColumnTreeItem* item = _FindLayerTreeViewItem(object);
	if (item != NULL)
		Select(IndexOf(item));
	else
		DeselectAll();
}

// #pragma mark -

// _Init
void
ObjectTreeView::_Init()
{
	AddColumn(new Column("Name", "name", 177,
		COLUMN_MOVABLE | COLUMN_VISIBLE));

	AddColumn(new Column("", "icon", 18,
		COLUMN_MOVABLE | COLUMN_VISIBLE));
}

// _HandleRenameSelectedItem
void
ObjectTreeView::_HandleRenameSelectedItem()
{
	_HandleRenameItem(CurrentSelection(0));
}

// _HandleRenameItem
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

		AutoReadLocker locker(fDocument);

		new TextViewPopup(frame, item->object->Name(), message, this);
	} catch (...) {
		delete message;
		item->object->RemoveReference();
	}
}

// _HandleRenameObject
void
ObjectTreeView::_HandleRenameObject(BMessage* message)
{
	BaseObject* object;
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
		// rename via command
		RenameObjectEdit* command = new (nothrow) RenameObjectEdit(object,
			name);
		fDocument->EditManager()->Perform(command, fEditContext);
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
ObjectTreeView::_ObjectAdded(Layer* layer, Object* object, int32 index)
{
//printf("ObjectTreeView::_ObjectAdded(%p, %p, %ld)\n", layer, object, index);
	if (!layer->HasObject(object))
		return;

	fIgnoreSelectionChanged = true;

	ObjectColumnTreeItem* parentItem = _FindLayerTreeViewItem(layer);

	ObjectColumnTreeItem* item = new ObjectColumnTreeItem(DefaultItemHeight(), object);
	item->Update();

	AddSubItem(parentItem, item, index);
	ExpandItem(item);

	if (Layer* subLayer = dynamic_cast<Layer*>(object))
		_RecursiveAddItems(subLayer, item);
	else if (Shape* shape = dynamic_cast<Shape*>(object))
		_AddPathItems(shape, item);

	fIgnoreSelectionChanged = false;
}

// _ObjectRemoved
void
ObjectTreeView::_ObjectRemoved(Layer* layer, Object* object, int32 index)
{
//printf("ObjectTreeView::_ObjectRemoved(%p, %p, %ld)\n", layer, object, index);
	ObjectColumnTreeItem* parentItem = _FindLayerTreeViewItem(layer);
//	ObjectColumnTreeItem* item = dynamic_cast<ObjectColumnTreeItem*>(
//		SubItemAt(parentItem, index));

	fIgnoreSelectionChanged = true;

	// TODO: A bug in DefaultColumnTreeModel already deleted the returned
	// item:
	RemoveSubItem(parentItem, index);

	fIgnoreSelectionChanged = false;
}

// _ObjectChanged
void
ObjectTreeView::_ObjectChanged(Layer* layer, Object* object, int32 index)
{
	ObjectColumnTreeItem* item = _FindLayerTreeViewItem(object);
	if (!item)
		return;
	item->Update();
	InvalidateItem(item);
}

//
// _ObjectSelected
void
ObjectTreeView::_ObjectSelected(BaseObject* object, bool selected)
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

// _PathAdded
void
ObjectTreeView::_PathAdded(Shape* shape, PathInstance* path, int32 index)
{
	PathInstanceRef ref(path);
//printf("ObjectTreeView::_ObjectAdded(%p, %p, %ld)\n", layer, object, index);
	if (!shape->Paths().Contains(ref))
		return;

	fIgnoreSelectionChanged = true;

	ObjectColumnTreeItem* parentItem = _FindLayerTreeViewItem(shape);

	ObjectColumnTreeItem* item = new ObjectColumnTreeItem(DefaultItemHeight(), path);
	item->Update();

	AddSubItem(parentItem, item, index);

	fIgnoreSelectionChanged = false;
}

// _PathRemoved
void
ObjectTreeView::_PathRemoved(Shape* shape, PathInstance* path, int32 index)
{
//printf("ObjectTreeView::_ObjectRemoved(%p, %p, %ld)\n", layer, object, index);
	ObjectColumnTreeItem* parentItem = _FindLayerTreeViewItem(shape);
//	ObjectColumnTreeItem* item = dynamic_cast<ObjectColumnTreeItem*>(
//		SubItemAt(parentItem, index));

	fIgnoreSelectionChanged = true;

	// TODO: A bug in DefaultColumnTreeModel already deleted the returned
	// item:
	RemoveSubItem(parentItem, index);

	fIgnoreSelectionChanged = false;
}

// _FindLayerTreeViewItem
ObjectColumnTreeItem*
ObjectTreeView::_FindLayerTreeViewItem(const BaseObject* object)
{
	int32 count = CountItems();
	for (int32 i = 0; i < count; i++) {
		ObjectColumnTreeItem* item = dynamic_cast<ObjectColumnTreeItem*>(
			ItemAt(i));
		if (item && item->object == object)
			return item;
	}
	return NULL;
}

// _RecursiveAddItems
void
ObjectTreeView::_RecursiveAddItems(Layer* layer,
	ObjectColumnTreeItem* layerItem)
{
	layer->AddListener(&fLayerObserver);

	float itemHeight = DefaultItemHeight();

	int32 count = layer->CountObjects();
	for (int32 i = 0; i < count; i++) {
		Object* object = layer->ObjectAtFast(i);

		ObjectColumnTreeItem* item = new ObjectColumnTreeItem(itemHeight, object);
		item->Update();

		if (layerItem != NULL)
			AddSubItem(layerItem, item, i);
		else
			AddItem(item, i);

		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer != NULL) {
			ExpandItem(item);
			_RecursiveAddItems(subLayer, item);
		}
		
		Shape* shape = dynamic_cast<Shape*>(object);
		if (shape != NULL)
			_AddPathItems(shape, item);
	}
}

// _AddPathItems
void
ObjectTreeView::_AddPathItems(Shape* shape,
	ObjectColumnTreeItem* layerItem)
{
	shape->AddShapeListener(&fShapeObserver);

	const PathList& paths = shape->Paths();
	float itemHeight = DefaultItemHeight();

	int32 count = paths.CountItems();
	for (int32 i = 0; i < count; i++) {
		const PathInstanceRef& path = paths.ItemAtFast(i);

		ObjectColumnTreeItem* item = new ObjectColumnTreeItem(itemHeight, path.Get());
		item->Update();

		if (layerItem != NULL)
			AddSubItem(layerItem, item, i);
		else
			AddItem(item, i);
	}
}

// _DropObjects
void
ObjectTreeView::_DropObjects(const BMessage& dragMessage, int32 count,
	Layer* insertionLayer, int32 subItemIndex, int32 itemIndex)
{
	Object** objects = new(std::nothrow) Object*[count];
	if (objects == NULL)
		return;

	int32 index = 0;
	for (int32 i = 0; i < count; i++) {
		BaseObject* baseObject;
		if (dragMessage.FindPointer("object", i,
				(void**)&baseObject) != B_OK) {
			delete[] objects;
			return;
		}
		Object* object = dynamic_cast<Object*>(baseObject);
		if (object != NULL)
			objects[index++] = object; 
	}

	count = index;
	MoveObjectsEdit* command = new(std::nothrow) MoveObjectsEdit(
		objects, count, insertionLayer, subItemIndex, fSelection);
	if (command == NULL) {
		delete[] objects;
		return;
	}

	fDocument->EditManager()->Perform(command, fEditContext);
}

// _DropPaths
void
ObjectTreeView::_DropPaths(const BMessage& dragMessage, int32 count,
	Shape* insertionShape, int32 subItemIndex, int32 itemIndex)
{
	PathInstance** paths = new(std::nothrow) PathInstance*[count];
	if (paths == NULL)
		return;

	int32 index = 0;
	for (int32 i = 0; i < count; i++) {
		BaseObject* baseObject;
		if (dragMessage.FindPointer("path", i,
				(void**)&baseObject) != B_OK) {
			delete[] paths;
			return;
		}
		PathInstance* path = dynamic_cast<PathInstance*>(baseObject);
		if (path != NULL)
			paths[index++] = path; 
	}

	count = index;
	MovePathsEdit* command = new(std::nothrow) MovePathsEdit(
		paths, count, insertionShape, subItemIndex, fSelection);
	if (command == NULL) {
		delete[] paths;
		return;
	}

	fDocument->EditManager()->Perform(command, fEditContext);
}
