#include "Window.h"

#include <Application.h>
#include <Bitmap.h>
#include <Box.h>
#include <GroupLayoutBuilder.h>
#include <LayoutUtils.h>
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
#include "IconOptionsControl.h"
//#include "LayerTreeModel.h"
#include "ObjectTreeView.h"
#include "PickToolState.h"
#include "RenderManager.h"
#include "ScrollView.h"
#include "TransformToolState.h"

enum {
	MSG_UNDO				= 'undo',
	MSG_REDO				= 'redo',
	MSG_SELECTION_CHANGED	= 'slch'
};


class SeparatorView : public BView {
public:
								SeparatorView(enum orientation orientation);
	virtual						~SeparatorView();

	virtual	BSize				MinSize();
	virtual	BSize				PreferredSize();
	virtual	BSize				MaxSize();
private:
			enum orientation	fOrientation;
};


SeparatorView::SeparatorView(enum orientation orientation)
	:
	BView("separator", 0),
	fOrientation(orientation)
{
	SetViewColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
		B_DARKEN_2_TINT));
}


SeparatorView::~SeparatorView()
{
}


BSize
SeparatorView::MinSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMinSize(), BSize(0, 0));
}


BSize
SeparatorView::MaxSize()
{
	BSize size(0, 0);
	if (fOrientation == B_VERTICAL)
		size.height = B_SIZE_UNLIMITED;
	else
		size.width = B_SIZE_UNLIMITED;

	return BLayoutUtils::ComposeSize(ExplicitMaxSize(), size);
}


BSize
SeparatorView::PreferredSize()
{
	BSize size(0, 0);
	if (fOrientation == B_VERTICAL)
		size.height = 10;
	else
		size.width = 10;

	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(), size);
}

// constructor
Window::Window(BRect frame, const char* title, Document* document,
			Layer* layer)
	:
	BWindow(frame, title, B_DOCUMENT_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS),
	fDocument(document),
	fRenderManager(NULL),
	fCommandStackListener(this),
//	fLayerTreeModel(new LayerTreeModel(fDocument)),
	fLayerObserver(this)
{
	SetLayout(new BGroupLayout(B_VERTICAL));

	// TODO: fix for when document == NULL

	BMenuBar* menuBar = new BMenuBar("main menu");
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

	fLayerTreeView = new ObjectTreeView(fDocument);

	Column* nameColumn = new Column("Name", "name", 177,
		COLUMN_MOVABLE | COLUMN_VISIBLE);
	fLayerTreeView->AddColumn(nameColumn);

	Column* iconColumn = new Column("", "icon", 18,
		COLUMN_MOVABLE | COLUMN_VISIBLE);
	fLayerTreeView->AddColumn(iconColumn);

//	fLayerTreeView->SetModel(fLayerTreeModel);
	fLayerTreeView->SetModel(new DefaultColumnTreeModel);
	ScrollView* objectTreeScrollView = new ScrollView(fLayerTreeView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL | SCROLL_HORIZONTAL_MAGIC
		| SCROLL_VERTICAL_MAGIC | SCROLL_VISIBLE_RECT_IS_CHILD_BOUNDS,
		"layer tree", B_WILL_DRAW | B_FRAME_EVENTS, B_PLAIN_BORDER,
		BORDER_TOP);
	objectTreeScrollView->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));
	objectTreeScrollView->SetExplicitMaxSize(BSize(250, B_SIZE_UNSET));

	fLayerTreeView->SetSelectionMessage(new BMessage(MSG_SELECTION_CHANGED));

	fRenderManager = new RenderManager(fDocument);
	// TODO: Check error
	fRenderManager->Init();
	fView = new CanvasView(fDocument, fRenderManager);
	ScrollView* canvasScrollView = new ScrollView(fView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL | SCROLL_NO_FRAME
		| SCROLL_VISIBLE_RECT_IS_CHILD_BOUNDS, "canvas",
		B_WILL_DRAW | B_FRAME_EVENTS, B_NO_BORDER);

	IconOptionsControl* iconBar = new IconOptionsControl();

	AddChild(BGroupLayoutBuilder(B_HORIZONTAL)
		.Add(BGroupLayoutBuilder(B_VERTICAL, 5)
			.Add(iconBar)
			.Add(objectTreeScrollView)
			.SetInsets(0, 5, 0, 0), 0.2
		)
		.Add(new SeparatorView(B_VERTICAL))
		.Add(canvasScrollView)
	);

	fView->MakeFocus(true);

	fPickState = new PickToolState(fView, layer, fDocument);
//	fView->SetState(fPickState);
TransformToolState* state = new TransformToolState(fView,
	BRect(150, 150, 280, 250));
Transformable t;
t.ScaleBy(BPoint(220, 200), 1.2, 1.5);
t.RotateBy(BPoint(200, 200), 10);
state->SetObjectToCanvasTransformation(t);
fView->SetState(state);
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

