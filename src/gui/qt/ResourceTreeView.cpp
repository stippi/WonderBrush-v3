/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>.
 * Copyright 2012, Ingo Weinhold <ingo_weinhold@gmx.de>.
 * All rights reserved.
 */


#include "ResourceTreeView.h"

#include <new>
#include <stdio.h>

#include <Bitmap.h>
#include <Window.h>

#include <QVBoxLayout>

#include "AbstractTreeModel.h"
#include "AutoDeleter.h"
#include "CommandStack.h"
#include "RenameObjectCommand.h"


enum {
	MSG_RENAME_SELECTED_ITEM	= 'rnit',
	MSG_OBJECT_SELECTED			= 'objs'
};


struct ResourceTreeView::TreeModel : AbstractTreeModel {
	enum {
		NAME_COLUMN = 0
	};

public:
	TreeModel(Document* document, Selection* selection, QObject* parent = NULL)
		:
		AbstractTreeModel(parent),
		fDocument(document),
		fSelection(selection)
	{
		SetHeaderName(NAME_COLUMN, Qt::Horizontal, QString::fromUtf8("Name"));
// TODO: Icon column -- better use decoration?
//		SetHeaderName(1, Qt::Horizontal, QString::fromUtf8(""));

//		SetDragSortable(true);
	}

	QModelIndex AddObject(const QModelIndex& parentIndex, int32 childIndex,
		BaseObject* object, bool notify)
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

	bool RemoveObject(BaseObject* object)
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

	bool ObjectChanged(BaseObject* object)
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

	QModelIndex ModelIndexFor(BaseObject* object) const
	{
		ObjectNode* node = fObjectNodes.value(object, NULL);
		return node != NULL ? ModelIndexOfNode(node) : QModelIndex();
	}

	BaseObject* ObjectForIndex(const QModelIndex& index) const
	{
		ObjectNode* node = dynamic_cast<ObjectNode*>(GetNode(index));
		return node != NULL ? node->GetObject() : NULL;
	}

protected:
	virtual Qt::ItemFlags NodeFlags(Node* node, int column) const
	{
#if 0
		if (node == fRootNode)
			return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
		return node->Flags(column);
#endif
		return node->Flags(column);
	}

	virtual bool SetNodeData(Node* node, int column, const QVariant& value)
	{
		ObjectNode* objectNode = dynamic_cast<ObjectNode*>(node);
		if (objectNode == NULL || column != NAME_COLUMN)
			return node->SetData(column, value);

		QString oldName = objectNode->Name();

		if (!node->SetData(column, value))
			return false;

		// The node name has been changed -- new rename the object as well.
		AutoWriteLocker locker(fDocument);

		BaseObject* object = objectNode->GetObject();
		BString newName = objectNode->Name();

		if (newName != object->Name()) {
			// rename via command
			RenameObjectCommand* command
				= new (std::nothrow) RenameObjectCommand(object, newName);
			if (command == NULL) {
				objectNode->SetName(oldName);
				return false;
			}

			fDocument->CommandStack()->Perform(command);
		}

		locker.Unlock();
		return true;
	}

#if 0
	virtual bool DropNodes(Node* _parentNode, int row,
		const QList<Node*>& nodes)
	{
		if (nodes.isEmpty())
			return false;

		// get the insertion layer
		ObjectNode* parentNode = dynamic_cast<ObjectNode*>(_parentNode);
		Layer* insertionLayer = parentNode != NULL
			? dynamic_cast<Layer*>(parentNode->GetObject())
			: fDocument->RootLayer();
		if (insertionLayer == NULL)
			return false;

		// dropping onto the layer item shall insert at the beginning
		if (row < 0)
			row = 0;

		int32 count = nodes.count();
		Object** objects = new(std::nothrow) Object*[count];
		if (objects == NULL)
			return false;
		ArrayDeleter<Object*> objectsDeleter(objects);

		for (int32 i = 0; i < count; i++) {
			ObjectNode* node = dynamic_cast<ObjectNode*>(nodes.at(i));
			if (node == NULL)
				return false;
			objects[i] = node->GetObject();
		}

		MoveObjectsCommand* command = new(std::nothrow) MoveObjectsCommand(
			objects, count, insertionLayer, row, fSelection);
		if (command == NULL)
			return false;
		objectsDeleter.Detach();

		fDocument->CommandStack()->Perform(command);
		return true;
	}
#endif

private:
	struct ObjectNode : ContainerNode {
		ObjectNode(BaseObject* object)
			:
			fObject(object),
			fIsLayer(dynamic_cast<Layer*>(object) != NULL)
		{
			Update();
		}

		BaseObject* GetObject() const
		{
			return fObject.Get();
		}

		const QString& Name() const
		{
			return fName;
		}

		void SetName(const QString& name)
		{
			fName = name;
		}

		void Update()
		{
			fName = QString::fromUtf8(fObject->Name());
			fIsLayer = dynamic_cast<Layer*>(fObject.Get()) != NULL;
		}

		virtual QVariant Data(int column) const
		{
			if (column == NAME_COLUMN)
				return fName;
			return QVariant();
		}

		virtual bool SetData(int column, const QVariant& value)
		{
			if (column != NAME_COLUMN || !value.canConvert<QString>())
				return false;
			QString newName = value.toString();
			if (newName.isEmpty())
				return false;

			fName = newName;
			return true;
		}

		virtual Qt::ItemFlags Flags(int /*column*/) const
		{
#if 0
			Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable
				| Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
			if (fIsLayer)
				flags |= Qt::ItemIsDropEnabled;
			return flags;
#endif
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable
				| Qt::ItemIsEditable;
		}

	private:
		Reference<BaseObject>	fObject;
		QString					fName;
		bool					fIsLayer;
	};

private:
	QHash<BaseObject*, ObjectNode*>	fObjectNodes;
	Document*						fDocument;
	Selection*						fSelection;
};


// #pragma mark - ResourceTreeView


// constructor
ResourceTreeView::ResourceTreeView(Document* document, Selection* selection)
	:
	BView("object tree view", 0),
	fTree(new QTreeView(this)),
	fTreeModel(new TreeModel(document, selection, this)),
	fDocument(document),
	fSelection(selection),
	fResourceObserver(this),
	fIgnoreSelectionChanged(false)
{
	fTree->setModel(fTreeModel);
	fTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
//	fTree->setDragEnabled(true);
//	fTree->setDragDropMode(QAbstractItemView::InternalMove);
//	fTree->setDropIndicatorShown(true);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->addWidget(fTree);

	connect(fTree->selectionModel(),
		SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
		SLOT(_SelectionChanged()));
}


// destructor
ResourceTreeView::~ResourceTreeView()
{
}

// AttachedToWindow
void
ResourceTreeView::AttachedToWindow()
{
	BView::AttachedToWindow();
//	Window()->AddShortcut('e', B_COMMAND_KEY,
//		new BMessage(MSG_RENAME_SELECTED_ITEM), this);

	fSelection->AddListener(this);

	if (fDocument->WriteLock()) {
		_AddItems(fDocument->GlobalResources(), QModelIndex());
		fDocument->WriteUnlock();
	}
}

// DetachedFromWindow
void
ResourceTreeView::DetachedFromWindow()
{
	fSelection->RemoveListener(this);
//	Window()->RemoveShortcut('e', B_COMMAND_KEY);
	BView::DetachedFromWindow();
}

// MouseDown
void
ResourceTreeView::MouseDown(BPoint where)
{
	MakeFocus(true);
	BView::MouseDown(where);
}

// KeyDown
void
ResourceTreeView::KeyDown(const char* bytes, int32 numBytes)
{
	switch (bytes[0]) {
		// TODO: Some re-configurable global short cut handling...
		case 'e':
		case 'E':
//			_HandleRenameSelectedItem();
			break;

		default:
			BView::KeyDown(bytes, numBytes);
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
			BView::MessageReceived(message);
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


// _HandleRenameSelectedItem
void
ResourceTreeView::_HandleRenameSelectedItem()
{
	QModelIndexList selectedIndices
		= fTree->selectionModel()->selectedIndexes();
	if (!selectedIndices.isEmpty())
		fTree->edit(selectedIndices.first());
}


// _ObjectAdded
void
ResourceTreeView::_ObjectAdded(const ResourceList* list, BaseObject* object,
	int32 index)
{
//printf("ResourceTreeView::_ObjectAdded(%p, %p, %ld)\n", layer, object, index);
	if (!list->HasObject(object))
		return;

	QModelIndex parentNodeIndex;

	fIgnoreSelectionChanged = true;

	QModelIndex nodeIndex
		= fTreeModel->AddObject(parentNodeIndex, index, object, true);
	if (nodeIndex.isValid())
		_ExpandItem(nodeIndex);

	fIgnoreSelectionChanged = false;
}


// _ObjectRemoved
void
ResourceTreeView::_ObjectRemoved(const ResourceList* list, BaseObject* object,
	int32 index)
{
	fIgnoreSelectionChanged = true;

	fTreeModel->RemoveObject(object);

	fIgnoreSelectionChanged = false;
}


// _ObjectChanged
void
ResourceTreeView::_ObjectChanged(const ResourceList* list, BaseObject* object,
	int32 index)
{
	fTreeModel->ObjectChanged(object);
}


// _ObjectSelected
void
ResourceTreeView::_ObjectSelected(BaseObject* object, bool selected)
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


// _AddItems
void
ResourceTreeView::_AddItems(const ResourceList& list,
	const QModelIndex& parentNodeIndex)
{
	int32 count = list.CountObjects();
	for (int32 i = 0; i < count; i++) {
		BaseObject* object = list.ObjectAtFast(i);

		QModelIndex nodeIndex
			= fTreeModel->AddObject(parentNodeIndex, i, object, true);
		if (!nodeIndex.isValid())
			continue;

		_ExpandItem(nodeIndex);
	}
}


void
ResourceTreeView::_ExpandItem(const QModelIndex& index)
{
	fTree->expand(index);
	int childCount = fTreeModel->rowCount(index);
	for (int i = 0; i < childCount; i++)
		_ExpandItem(fTreeModel->index(i, 0, index));
}


void
ResourceTreeView::_SelectionChanged()
{
	// Do not mess with the selection while adding/removing items.
	if (fIgnoreSelectionChanged)
		return;

	// Sync the selections
	QSet<BaseObject*> selectedObjects;
	foreach (const QModelIndex& index,
		fTree->selectionModel()->selectedIndexes()) {
		if (BaseObject* object = fTreeModel->ObjectForIndex(index))
			selectedObjects.insert(object);
	}

	if (selectedObjects.isEmpty()) {
		fSelection->DeselectAll(this);
		return;
	}

	bool extend = false;
	foreach (BaseObject* object, selectedObjects) {
		fSelection->Select(Selectable(object), this, extend);
		extend = true;
	}
}
