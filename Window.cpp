#include <Application.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Message.h>
#include <String.h>

#include "Window.h"

#include "App.h"
#include "Column.h"
#include "ColumnTreeView.h"
#include "CommandStack.h"
#include "DefaultColumnTreeModel.h"
#include "Document.h"
//#include "LayerTreeModel.h"
#include "PickToolState.h"
#include "RenderManager.h"
#include "ScrollView.h"
#include "TextColumnTreeItem.h"
#include "View.h"

enum {
	MSG_UNDO = 'undo',
	MSG_REDO = 'redo'
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
	fLayerTreeView = new ColumnTreeView(frame);

	Column* column1 = new Column("Column 1", "column1", 150,
								  COLUMN_MOVABLE | COLUMN_VISIBLE
								  | COLUMN_SORT_KEYABLE);
	fLayerTreeView->AddColumn(column1);

//	fLayerTreeView->SetModel(fLayerTreeModel);
	fLayerTreeView->SetModel(new DefaultColumnTreeModel);
	ScrollView* scrollView = new ScrollView(fLayerTreeView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL | SCROLL_HORIZONTAL_MAGIC
		| SCROLL_VERTICAL_MAGIC, frame, "layer tree", 0, 0);

	AddChild(scrollView);

	frame.left = frame.right + 1;
	frame.right = Bounds().right;
	fRenderManager = new RenderManager(fDocument);
	fView = new View(frame, fDocument, fRenderManager);
	AddChild(fView);
	fView->MakeFocus(true);

	fView->SetState(new PickToolState(fView, layer, fDocument));
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

		case LayerObserver::MSG_OBJECT_ADDED:
		case LayerObserver::MSG_OBJECT_REMOVED:
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
					else
						_ObjectRemoved(layer, object, index);
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
	be_app->PostMessage(B_QUIT_REQUESTED);
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

class ObjectColumnTreeItem : public TextColumnTreeItem {
 public:
	Object*	object;

			ObjectColumnTreeItem(float height, Object* object)
				: TextColumnTreeItem(height)
				, object(object)
			{
			}
	virtual	~ObjectColumnTreeItem()
			{
			}
};

// _ObjectAdded
void
Window::_ObjectAdded(Layer* layer, Object* object, int32 index)
{
	if (!layer->HasObject(object))
		return;

	ObjectColumnTreeItem* parentItem = _FindLayerTreeViewItem(layer);

	ObjectColumnTreeItem* item = new ObjectColumnTreeItem(20, object);
	item->SetText("Object", 0);

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
		item->SetText("Object", 0);

		if (layerItem)
			fLayerTreeView->AddSubItem(layerItem, item, i);
		else
			fLayerTreeView->AddItem(item, i);
		fLayerTreeView->ExpandItem(item);

		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer) {
printf("sub layer!\n");
			_RecursiveAddItems(subLayer, item);
		}
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

