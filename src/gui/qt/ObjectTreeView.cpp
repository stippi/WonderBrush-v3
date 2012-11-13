/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>.
 * Copyright 2012, Ingo Weinhold <ingo_weinhold@gmx.de>.
 * All rights reserved.
 */


#include "ObjectTreeView.h"

#include <new>
#include <stdio.h>

#include <Bitmap.h>
#include <Window.h>

#include <QHash>
#include <QVBoxLayout>

#include "AbstractTreeModel.h"
#include "AutoDeleter.h"
#include "CommandStack.h"
#include "Document.h"
#include "MoveObjectsCommand.h"
#include "Object.h"
#include "RenameObjectCommand.h"


enum {
	MSG_RENAME_OBJECT			= 'rnoj',
	MSG_RENAME_SELECTED_ITEM	= 'rnit',
	MSG_DRAG_SORT_OBJECTS		= 'drgo',
	MSG_OBJECT_SELECTED			= 'objs'
};


struct ObjectTreeView::TreeModel : AbstractTreeModel {
	TreeModel(QObject* parent = NULL)
		:
		AbstractTreeModel(parent)
	{
		SetHeaderName(0, Qt::Horizontal, QString::fromUtf8("Name"));
// TODO: Icon column -- better use decoration?
//		SetHeaderName(1, Qt::Horizontal, QString::fromUtf8(""));
	}

	QModelIndex AddObject(const QModelIndex& parentIndex, int32 childIndex,
		Object* object, bool notify)
	{
		Node* parentNode = GetNode(parentIndex);
		if (parentNode == NULL || childIndex < 0
			|| childIndex > parentNode->CountChildren()) {
			return QModelIndex();
		}

		ObjectNode* node = new(std::nothrow) ObjectNode(object);
		if (node == NULL)
			return QModelIndex();

		fObjectNodes.insert(object, node);

		if (notify) {
			beginInsertRows(ModelIndexOfNode(parentNode), childIndex,
				childIndex);
		}

		parentNode->InsertChild(childIndex, node);

		if (notify)
			endInsertRows();

		return ModelIndexOfNode(node);
	}

	bool RemoveObject(Object* object)
	{
		ObjectNode* node = fObjectNodes.value(object);
		if (node == NULL)
			return false;

		QModelIndex parentNodeIndex = ModelIndexOfNode(node->Parent());
		int childIndex = node->Index();
		beginRemoveRows(parentNodeIndex, childIndex, childIndex);
		node->Parent()->RemoveChild(childIndex);
		endRemoveRows();

		delete node;
		return true;
	}

	bool ObjectChanged(Object* object)
	{
		ObjectNode* node = fObjectNodes.value(object);
		if (node == NULL)
			return false;

		node->Update();

		QModelIndex parentNodeIndex = ModelIndexOfNode(node->Parent());
		QModelIndex childNodeIndex = index(node->Index(), 0, parentNodeIndex);
		dataChanged(childNodeIndex, childNodeIndex);

		return true;
	}

	QModelIndex ModelIndexFor(Object* object) const
	{
		ObjectNode* node = fObjectNodes.value(object, NULL);
		return node != NULL ? ModelIndexOfNode(node) : QModelIndex();
	}

	Object* ObjectForIndex(const QModelIndex& index) const
	{
		ObjectNode* node = dynamic_cast<ObjectNode*>(GetNode(index));
		return node != NULL ? node->GetObject() : NULL;
	}

private:
	struct ObjectNode : ContainerNode {
		ObjectNode(Object* object)
			:
			fObject(object)
		{
			Update();
		}

		Object* GetObject() const
		{
			return fObject;
		}

		void Update()
		{
			fName = QString::fromUtf8(fObject->Name());
		}

		virtual QVariant Data(int column) const
		{
			if (column == 0)
				return fName;
			return QVariant();
		}

		virtual Qt::ItemFlags Flags(int /*column*/) const
		{
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		}

	private:
		Object*	fObject;
		QString	fName;
	};

private:
	QHash<Object*, ObjectNode*>	fObjectNodes;
};


// constructor
ObjectTreeView::ObjectTreeView(Document* document, Selection* selection)
	:
	BView("object tree view", 0),
	fTree(new QTreeView(this)),
	fTreeModel(new TreeModel(this)),
	fDocument(document),
	fSelection(selection),
	fLayerObserver(this),
	fIgnoreSelectionChanged(false)
{
	fTree->setModel(fTreeModel);
	fTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->addWidget(fTree);

	connect(fTree->selectionModel(),
		SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
		SLOT(_SelectionChanged()));
}


// destructor
ObjectTreeView::~ObjectTreeView()
{
}

// AttachedToWindow
void
ObjectTreeView::AttachedToWindow()
{
	BView::AttachedToWindow();
#if 0
	Window()->AddShortcut('e', B_COMMAND_KEY,
		new BMessage(MSG_RENAME_SELECTED_ITEM), this);
#endif

	fSelection->AddListener(this);

	if (fDocument->WriteLock()) {
		_RecursiveAddItems(fDocument->RootLayer(), QModelIndex());
		fDocument->WriteUnlock();
	}
}

// DetachedFromWindow
void
ObjectTreeView::DetachedFromWindow()
{
	fSelection->RemoveListener(this);
//	Window()->RemoveShortcut('e', B_COMMAND_KEY);
	BView::DetachedFromWindow();
}

// MouseDown
void
ObjectTreeView::MouseDown(BPoint where)
{
	MakeFocus(true);
	BView::MouseDown(where);
}

// KeyDown
void
ObjectTreeView::KeyDown(const char* bytes, int32 numBytes)
{
	switch (bytes[0]) {
		// TODO: Some re-configurable global short cut handling...
		case 'e':
		case 'E':
//			_HandleRenameSelectedItem();
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
			BView::KeyDown(bytes, numBytes);
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
//			_HandleRenameObject(message);
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

		case MSG_OBJECT_SELECTED:
		{
			Object* object;
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
			BView::MessageReceived(message);
	}
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

	Object* object = dynamic_cast<Object*>(selectable.Get());
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

	Object* object = dynamic_cast<Object*>(selectable.Get());
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
ObjectTreeView::SelectItem(Object* object)
{
	QModelIndex modelIndex = fTreeModel->ModelIndexFor(object);
	if (modelIndex.isValid()) {
		fTree->selectionModel()->select(modelIndex,
			QItemSelectionModel::ClearAndSelect);
	} else
		fTree->selectionModel()->clear();
}


// _HandleRenameSelectedItem
void
ObjectTreeView::_HandleRenameSelectedItem()
{
//	_HandleRenameItem(CurrentSelection(0));
}


// _HandleRenameItem
void
ObjectTreeView::_HandleRenameItem(int32 index)
{
#if 0
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
#endif
}


// _ObjectAdded
void
ObjectTreeView::_ObjectAdded(Layer* layer, Object* object, int32 index)
{
	if (!layer->HasObject(object))
		return;

	QModelIndex parentNodeIndex = fTreeModel->ModelIndexFor(layer);

	fIgnoreSelectionChanged = true;

	QModelIndex nodeIndex
		= fTreeModel->AddObject(parentNodeIndex, index, object, true);
	if (nodeIndex.isValid()) {
		_ExpandItem(nodeIndex);
		if (Layer* subLayer = dynamic_cast<Layer*>(object))
			_RecursiveAddItems(subLayer, nodeIndex);
	}

	fIgnoreSelectionChanged = false;
}


// _ObjectRemoved
void
ObjectTreeView::_ObjectRemoved(Layer* layer, Object* object, int32 index)
{
	fIgnoreSelectionChanged = true;

	fTreeModel->RemoveObject(object);

	fIgnoreSelectionChanged = false;
}


// _ObjectChanged
void
ObjectTreeView::_ObjectChanged(Layer* layer, Object* object, int32 index)
{
	fTreeModel->ObjectChanged(object);
}


// _ObjectSelected
void
ObjectTreeView::_ObjectSelected(Object* object, bool selected)
{
	fIgnoreSelectionChanged = true;

	QModelIndex modelIndex = fTreeModel->ModelIndexFor(object);
	if (modelIndex.isValid()) {
		fTree->selectionModel()->select(modelIndex,
			selected ? QItemSelectionModel::Select
				: QItemSelectionModel::Deselect);
	}

	fIgnoreSelectionChanged = false;
}


// _RecursiveAddItems
void
ObjectTreeView::_RecursiveAddItems(Layer* layer,
	const QModelIndex& layerNodeIndex)
{
	layer->AddListener(&fLayerObserver);

	int32 count = layer->CountObjects();
	for (int32 i = 0; i < count; i++) {
		Object* object = layer->ObjectAtFast(i);
		QModelIndex nodeIndex
			= fTreeModel->AddObject(layerNodeIndex, i, object, true);
		if (!nodeIndex.isValid())
			continue;

		_ExpandItem(nodeIndex);

		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer != NULL)
			_RecursiveAddItems(subLayer, nodeIndex);
	}
}


void
ObjectTreeView::_ExpandItem(const QModelIndex& index)
{
	fTree->expand(index);
	int childCount = fTreeModel->rowCount(index);
	for (int i = 0; i < childCount; i++)
		_ExpandItem(fTreeModel->index(i, 0, index));
}


void
ObjectTreeView::_SelectionChanged()
{
	// Do not mess with the selection while adding/removing items.
	if (fIgnoreSelectionChanged)
		return;

	// Sync the selections
	QSet<Object*> selectedObjects;
	foreach (const QModelIndex& index,
		fTree->selectionModel()->selectedIndexes()) {
		if (Object* object = fTreeModel->ObjectForIndex(index))
			selectedObjects.insert(object);
	}

	if (selectedObjects.isEmpty()) {
		fSelection->DeselectAll(this);
		return;
	}

	bool extend = false;
	foreach (Object* object, selectedObjects) {
		fSelection->Select(Selectable(object), this, extend);
		extend = true;
	}
}
