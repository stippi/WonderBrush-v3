#include "Window.h"

#include <Application.h>
#include <Bitmap.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Message.h>
#include <ScrollBar.h>
#include <String.h>

#include "App.h"
#include "CanvasView.h"
#include "Column.h"
#include "CommandStack.h"
#include "DefaultColumnTreeModel.h"
#include "Document.h"
//#include "LayerTreeModel.h"
#include "ObjectTreeView.h"
#include "PickToolState.h"
#include "RenderManager.h"
#include "ScrollView.h"

enum {
	MSG_UNDO				= 'undo',
	MSG_REDO				= 'redo',
	MSG_SELECTION_CHANGED	= 'slch'
};

// constructor
Window::Window(BRect frame, const char* title, Document* document,
			Layer* layer)
	: BWindow(frame, title,
		B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, B_ASYNCHRONOUS_CONTROLS)
	, fDocument(document)
	, fRenderManager(NULL)
	, fCommandStackListener(this)
//	, fLayerTreeModel(new LayerTreeModel(fDocument))
	, fLayerObserver(this)
{
	// TODO: fix for when document == NULL

	frame.OffsetTo(B_ORIGIN);
	frame.bottom = 15;
	BMenuBar* menuBar = new BMenuBar(frame, "main menu");
	BMenu* fileMenu = new BMenu("File");
	menuBar->AddItem(fileMenu);
	BMenuItem* newWindowMI = new BMenuItem("New Window",
		new BMessage(MSG_NEW_WINDOW), 'N');
	fileMenu->AddItem(newWindowMI);
	fileMenu->AddItem(new BMenuItem("Quit",
		new BMessage(B_QUIT_REQUESTED), 'Q'));

	BMenu* editMenu = new BMenu("Edit");
	BMessage* message = new BMessage(MSG_UNDO);
	fUndoMI = new BMenuItem("Undo", message);
	editMenu->AddItem(fUndoMI);
	message = new BMessage(MSG_REDO);
	fRedoMI = new BMenuItem("Undo", message);
	editMenu->AddItem(fRedoMI);
	menuBar->AddItem(editMenu);

	AddChild(menuBar);
	fileMenu->SetTargetForItems(this);
	editMenu->SetTargetForItems(this);
	newWindowMI->SetTarget(be_app);

	menuBar->ResizeToPreferred();
	frame = Bounds();
	frame.top = menuBar->Frame().bottom + 1;
	frame.right = 200;
	fLayerTreeView = new ObjectTreeView(frame, fDocument);

	Column* nameColumn = new Column("Name", "name", 177,
		COLUMN_MOVABLE | COLUMN_VISIBLE);
	fLayerTreeView->AddColumn(nameColumn);

	Column* iconColumn = new Column("", "icon", 18,
		COLUMN_MOVABLE | COLUMN_VISIBLE);
	fLayerTreeView->AddColumn(iconColumn);


//	fLayerTreeView->SetModel(fLayerTreeModel);
	fLayerTreeView->SetModel(new DefaultColumnTreeModel);
	ScrollView* scrollView = new ScrollView(fLayerTreeView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL | SCROLL_HORIZONTAL_MAGIC
		| SCROLL_VERTICAL_MAGIC | SCROLL_VISIBLE_RECT_IS_CHILD_BOUNDS, 
		frame, "layer tree", B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM,
		B_WILL_DRAW | B_FRAME_EVENTS);

	AddChild(scrollView);

	fLayerTreeView->SetSelectionMessage(new BMessage(MSG_SELECTION_CHANGED));

	frame.left = frame.right + 1;
	frame.right = Bounds().right;
	frame.right -= B_V_SCROLL_BAR_WIDTH;
	frame.bottom -= B_H_SCROLL_BAR_HEIGHT;
	fRenderManager = new RenderManager(fDocument);
	// TODO: Check error
	fRenderManager->Init();
	fView = new CanvasView(frame, fDocument, fRenderManager);
	frame.right += B_V_SCROLL_BAR_WIDTH;
	frame.bottom += B_H_SCROLL_BAR_HEIGHT;
	scrollView = new ScrollView(fView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL | SCROLL_NO_FRAME
		| SCROLL_VISIBLE_RECT_IS_CHILD_BOUNDS, frame, "canvas",
		B_FOLLOW_ALL, B_WILL_DRAW | B_FRAME_EVENTS);

	AddChild(scrollView);
	fView->MakeFocus(true);

	fPickState = new PickToolState(fView, layer, fDocument);
	fView->SetState(fPickState);
	fView->SetCommandStack(fDocument->CommandStack());

	fDocument->CommandStack()->AddListener(&fCommandStackListener);
	_ObjectChanged(fDocument->CommandStack());
	_RecursiveAddListener(fDocument->RootLayer());
	_RecursiveAddItems(fDocument->RootLayer(), NULL);
}

// destructor
Window::~Window()
{
	_RecursiveRemoveListener(fDocument->RootLayer());
	fDocument->CommandStack()->RemoveListener(&fCommandStackListener);
	delete fRenderManager;
//	delete fLayerTreeModel;
}

// MessageReceived
void
Window::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_UNDO:
			fDocument->CommandStack()->Undo();
			break;
		case MSG_REDO:
			fDocument->CommandStack()->Redo();
			break;

		case MSG_OBJECT_CHANGED: {
			Notifier* notifier;
			if (message->FindPointer("object", (void**)&notifier) == B_OK)
				_ObjectChanged(notifier);
			break;
		}

		case MSG_SELECTION_CHANGED:
		{
			int32 index;
			if (message->FindInt32("index", &index) == B_OK) {
				ObjectColumnTreeItem* item
					= dynamic_cast<ObjectColumnTreeItem*>(
						fLayerTreeView->ItemAt(index));
				if (item) {
					Object* object = item->object;
					Layer* layer = object->Parent();
					if (!layer)
						layer = fDocument->RootLayer();
					fPickState->SetObject(layer, object);
				}
			}
			break;
		}

		case PickToolState::MSG_OBJECT_PICKED:
		{
			Object* object;
			if (message->FindPointer("object", (void**)&object) == B_OK) {
				ColumnTreeItem* item = _FindLayerTreeViewItem(object);
				if (item)
					fLayerTreeView->Select(fLayerTreeView->IndexOf(item));
					//item->SetSelected(true);
				else
					fLayerTreeView->DeselectAll();
			}
		}

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
		default:
			BWindow::MessageReceived(message);
	}
}

// QuitRequested
bool
Window::QuitRequested()
{
	be_app->PostMessage(MSG_WINDOW_QUIT);
	return true;
}

// #pragma mark -

// SetDocument
void
Window::SetDocument(Document* document)
{
	fDocument = document;
	// TODO: handle to View
}

// #pragma mark -

// _ObjectChanged
void
Window::_ObjectChanged(const Notifier* object)
{
	if (!fDocument)
		return;

	if (object == fDocument->CommandStack()) {
		// relable Undo item and update enabled status
		BString label("Undo");
		fUndoMI->SetEnabled(fDocument->CommandStack()->GetUndoName(label));
		if (fUndoMI->IsEnabled())
			fUndoMI->SetLabel(label.String());
		else
			fUndoMI->SetLabel("<nothing to undo>");

		// relable Redo item and update enabled status
		label.SetTo("Redo");
		fRedoMI->SetEnabled(fDocument->CommandStack()->GetRedoName(label));
		if (fRedoMI->IsEnabled())
			fRedoMI->SetLabel(label.String());
		else
			fRedoMI->SetLabel("<nothing to redo>");
	}
}

// _ObjectAdded
void
Window::_ObjectAdded(Layer* layer, Object* object, int32 index)
{
	if (!layer->HasObject(object))
		return;

	ObjectColumnTreeItem* parentItem = _FindLayerTreeViewItem(layer);

	ObjectColumnTreeItem* item = new ObjectColumnTreeItem(20, object);
	item->Update();

	fLayerTreeView->AddSubItem(parentItem, item, index);
	fLayerTreeView->ExpandItem(item);

	if (Layer* subLayer = dynamic_cast<Layer*>(object))
		_RecursiveAddItems(subLayer, item);
}

// _ObjectRemoved
void
Window::_ObjectRemoved(Layer* layer, Object* object, int32 index)
{
}

// _ObjectChanged
void
Window::_ObjectChanged(Layer* layer, Object* object, int32 index)
{
	ObjectColumnTreeItem* item = _FindLayerTreeViewItem(object);
	if (!item)
		return;
	item->Update();
	fLayerTreeView->InvalidateItem(item);
}

// _FindLayerTreeViewItem
ObjectColumnTreeItem*
Window::_FindLayerTreeViewItem(const Object* object)
{
	int32 count = fLayerTreeView->CountItems();
	for (int32 i = 0; i < count; i++) {
		ObjectColumnTreeItem* item = dynamic_cast<ObjectColumnTreeItem*>(
			fLayerTreeView->ItemAt(i));
		if (item && item->object == object)
			return item;
	}
	return NULL;
}

// _RecursiveAddItems
void
Window::_RecursiveAddItems(Layer* layer, ObjectColumnTreeItem* layerItem)
{
	int32 count = layer->CountObjects();
	for (int32 i = 0; i < count; i++) {
		Object* object = layer->ObjectAtFast(i);
		
		ObjectColumnTreeItem* item = new ObjectColumnTreeItem(20, object);
		item->Update();

		if (layerItem)
			fLayerTreeView->AddSubItem(layerItem, item, i);
		else
			fLayerTreeView->AddItem(item, i);
		fLayerTreeView->ExpandItem(item);

		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer)
			_RecursiveAddItems(subLayer, item);
	}
}

// _RecursiveAddListener
void
Window::_RecursiveAddListener(Layer* layer)
{
	// the document is locked and/or this is executed from within
	// a synchronous notification
	int32 count = layer->CountObjects();
	for (int32 i = 0; i < count; i++) {
		Object* object = layer->ObjectAtFast(i);
		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer)
			_RecursiveAddListener(subLayer);
	}

	layer->AddListener(&fLayerObserver);
}

// _RecursiveRemoveListener
void
Window::_RecursiveRemoveListener(Layer* layer)
{
	// the document is locked and/or this is executed from within
	// a synchronous notification
	int32 count = layer->CountObjects();
	for (int32 i = 0; i < count; i++) {
		Object* object = layer->ObjectAtFast(i);
		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer)
			_RecursiveRemoveListener(subLayer);
	}

	layer->RemoveListener(&fLayerObserver);
}

