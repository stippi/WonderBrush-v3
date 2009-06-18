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
#include "IconButton.h"
#include "IconOptionsControl.h"
//#include "LayerTreeModel.h"
#include "ObjectTreeView.h"
#include "PickTool.h"
#include "TransformTool.h"
#include "PickToolState.h"
#include "RenderManager.h"
#include "ScrollView.h"

enum {
	MSG_UNDO				= 'undo',
	MSG_REDO				= 'redo',
	MSG_SELECTION_CHANGED	= 'slch',
	MSG_SET_TOOL			= 'sltl'
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

	fToolIconControl = new IconOptionsControl();

	AddChild(BGroupLayoutBuilder(B_HORIZONTAL)
		.Add(BGroupLayoutBuilder(B_VERTICAL, 5)
			.Add(fToolIconControl)
			.Add(objectTreeScrollView)
			.SetInsets(0, 5, 0, 0), 0.2
		)
		.Add(new SeparatorView(B_VERTICAL))
		.Add(canvasScrollView)
	);

	fView->MakeFocus(true);

	_InitTools();

	fView->SetCommandStack(fDocument->CommandStack());

	fDocument->CommandStack()->AddListener(&fCommandStackListener);
	_ObjectChanged(fDocument->CommandStack());
	Layer::AddListenerRecursive(fDocument->RootLayer(), &fLayerObserver);
}

// destructor
Window::~Window()
{
	Layer::RemoveListenerRecursive(fDocument->RootLayer(), &fLayerObserver);
	fDocument->CommandStack()->RemoveListener(&fCommandStackListener);
	delete fRenderManager;
//	delete fLayerTreeModel;

	fView->SetState(NULL);
	for (int32 i = fTools.CountItems() - 1; i >= 0; i--)
		delete (Tool*)fTools.ItemAtFast(i);
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

		case MSG_SET_TOOL: {
			int32 index;
			if (message->FindInt32("tool", &index) == B_OK) {
				if (Tool* tool = (Tool*)fTools.ItemAt(index))
					fView->SetState(tool->ToolViewState(fView, fDocument));
			}
			break;
		}

		case PickToolState::MSG_OBJECT_PICKED:
		{
			Object* object;
			if (message->FindPointer("object", (void**)&object) == B_OK)
				fLayerTreeView->SelectItem(object);
		}

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

// AddTool
void
Window::AddTool(Tool* tool)
{
	if (tool == NULL)
		return;

	int32 count = fTools.CountItems();
		// check the count before adding tool

	if (!fTools.AddItem((void*)tool)) {
		delete tool;
		return;
	}

	// add the tools icon
	IconButton* icon = tool->Icon();
	BMessage* message = new BMessage(MSG_SET_TOOL);
	message->AddInt32("tool", count);
	icon->SetMessage(message);
	fToolIconControl->AddOption(icon);

//	if (count == 0) {
//		// this was the first tool
//		fTimelineView->SetTool(tool);
//	}
}

// #pragma mark -

// _InitTools
void
Window::_InitTools()
{
	// create canvas tools
	PickTool* pickTool = new(std::nothrow) PickTool();
	// TODO: Remove test code...
	fPickState = dynamic_cast<PickToolState*>(
		pickTool->ToolViewState(fView, fDocument));
	fView->SetState(fPickState);

	AddTool(pickTool);
	AddTool(new(std::nothrow) TransformTool());
}

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
