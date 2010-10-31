/*
 * Copyright 2007-2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "Window.h"

#include <Application.h>
#include <Bitmap.h>
#include <Box.h>
#include <CardLayout.h>
#include <LayoutBuilder.h>
#include <LayoutUtils.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Message.h>
#include <ScrollBar.h>
#include <SeparatorView.h>
#include <String.h>
#include <TabView.h>

#include "BrushTool.h"
#include "CanvasView.h"
#include "CommandStack.h"
#include "DefaultColumnTreeModel.h"
#include "Document.h"
#include "IconButton.h"
#include "IconOptionsControl.h"
#include "InspectorView.h"
//#include "LayerTreeModel.h"
#include "NavigatorView.h"
#include "ObjectTreeView.h"
#include "PickTool.h"
#include "ToolConfigView.h"
#include "TransformTool.h"
#include "RenderManager.h"
#include "ResourceTreeView.h"
#include "ScrollView.h"
#include "WonderBrush.h"

enum {
	MSG_UNDO				= 'undo',
	MSG_REDO				= 'redo',
	MSG_SET_TOOL			= 'sltl'
};


// constructor
Window::Window(BRect frame, const char* title, Document* document,
			Layer* layer)
	: BWindow(frame, title, B_DOCUMENT_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS)
	, fDocument(document)
	, fRenderManager(NULL)
	, fCommandStackListener(this)
//	, fLayerTreeModel(new LayerTreeModel(fDocument))
{
	// TODO: fix for when document == NULL

	BMenuBar* menuBar = new BMenuBar("main menu");
	fFileMenu = new BMenu("File");
	menuBar->AddItem(fFileMenu);
	BMenuItem* newWindowMI = new BMenuItem("New Window",
		new BMessage(MSG_NEW_WINDOW), 'N');
	fFileMenu->AddItem(newWindowMI);
	fFileMenu->AddItem(new BMenuItem("Quit",
		new BMessage(B_QUIT_REQUESTED), 'Q'));

	fEditMenu = new BMenu("Edit");
	menuBar->AddItem(fEditMenu);
	BMessage* message = new BMessage(MSG_UNDO);
	fUndoMI = new BMenuItem("Undo", message, 'Y');
	fEditMenu->AddItem(fUndoMI);
	message = new BMessage(MSG_REDO);
	fRedoMI = new BMenuItem("Redo", message, 'Y', B_SHIFT_KEY);
	fEditMenu->AddItem(fRedoMI);

	BMenuBar* objectMenuBar = new BMenuBar("object menu");
	fObjectMenu = new BMenu("Object");
	objectMenuBar->AddItem(fObjectMenu);

	BMenuBar* resourceMenuBar = new BMenuBar("resource menu");
	fResourceMenu = new BMenu("Resource");
	resourceMenuBar->AddItem(fResourceMenu);

	BMenuBar* propertyMenuBar = new BMenuBar("property menu");
	fPropertyMenu = new BMenu("Property");
	propertyMenuBar->AddItem(fPropertyMenu);

	fLayerTreeView = new ObjectTreeView(fDocument, &fSelection);
//	fLayerTreeView->SetModel(fLayerTreeModel);
	fLayerTreeView->SetModel(new DefaultColumnTreeModel);
	ScrollView* objectTreeScrollView = new ScrollView(fLayerTreeView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL/* | SCROLL_HORIZONTAL_MAGIC
		| SCROLL_VERTICAL_MAGIC*/ | SCROLL_VISIBLE_RECT_IS_CHILD_BOUNDS,
		"layer tree", B_WILL_DRAW | B_FRAME_EVENTS, B_PLAIN_BORDER,
		BORDER_BOTTOM);

	fResourceTreeView = new ResourceTreeView(fDocument, &fSelection);
	fResourceTreeView->SetModel(new DefaultColumnTreeModel);
	ScrollView* resourceTreeScrollView = new ScrollView(fResourceTreeView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL/* | SCROLL_HORIZONTAL_MAGIC
		| SCROLL_VERTICAL_MAGIC*/ | SCROLL_VISIBLE_RECT_IS_CHILD_BOUNDS,
		"layer tree", B_WILL_DRAW | B_FRAME_EVENTS, B_PLAIN_BORDER,
		BORDER_BOTTOM);

	fRenderManager = new RenderManager(fDocument);
	// TODO: Check error
	fRenderManager->Init();
	fView = new CanvasView(fDocument, fRenderManager);
	ScrollView* canvasScrollView = new ScrollView(fView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL | SCROLL_NO_FRAME
		| SCROLL_VISIBLE_RECT_IS_CHILD_BOUNDS, "canvas",
		B_WILL_DRAW | B_FRAME_EVENTS, B_NO_BORDER);

	BRect toolIconBounds(0, 0, 21, 21);

	// File icon group
	IconButton* newButton = new IconButton("new", 0);
	newButton->SetIcon(201);
	newButton->TrimIcon(toolIconBounds);
	IconButton* openButton = new IconButton("open", 0);
	openButton->SetIcon(202);
	openButton->TrimIcon(toolIconBounds);
	IconButton* saveButton = new IconButton("save", 0);
	saveButton->SetIcon(204);
	saveButton->TrimIcon(toolIconBounds);
	IconButton* exportButton = new IconButton("export", 0);
	exportButton->SetIcon(203);
	exportButton->TrimIcon(toolIconBounds);
	IconButton* closeButton = new IconButton("close", 0);
	closeButton->SetIcon(204);
	closeButton->TrimIcon(toolIconBounds);
	BGroupView* fileIconGroup = new BGroupView(B_HORIZONTAL, 0.0f);
	BLayoutBuilder::Group<>(fileIconGroup->GroupLayout())
		.Add(newButton)
		.Add(new BSeparatorView(B_VERTICAL))
		.Add(openButton)
		.Add(saveButton)
		.Add(exportButton)
		.Add(new BSeparatorView(B_VERTICAL))
		.Add(closeButton)
		.SetInsets(5, 5, 5, 5)
	;
	fileIconGroup->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	// Undo/Redo icon group
	fUndoIcon = new IconButton("undo", 0, NULL, new BMessage(MSG_UNDO));
	fUndoIcon->SetIcon(301);
	fUndoIcon->TrimIcon(toolIconBounds);
	fRedoIcon = new IconButton("redo", 0, NULL, new BMessage(MSG_REDO));
	fRedoIcon->SetIcon(302);
	fRedoIcon->TrimIcon(toolIconBounds);
	fConfirmIcon = new IconButton("confirm", 0);
	fConfirmIcon->SetIcon(303);
	fConfirmIcon->TrimIcon(toolIconBounds);
	fCancelIcon = new IconButton("cancel", 0);
	fCancelIcon->SetIcon(304);
	fCancelIcon->TrimIcon(toolIconBounds);

	fUndoIcon->SetEnabled(false);
	fRedoIcon->SetEnabled(false);
	fConfirmIcon->SetEnabled(false);
	fCancelIcon->SetEnabled(false);

	BGroupView* undoIconGroup = new BGroupView(B_HORIZONTAL, 0.0f);
	BLayoutBuilder::Group<>(undoIconGroup->GroupLayout())
		.Add(fUndoIcon)
		.Add(fRedoIcon)
		.Add(new BSeparatorView(B_VERTICAL))
		.Add(fConfirmIcon)
		.Add(fCancelIcon)
		.SetInsets(5, 5, 5, 5)
	;

	fToolIconControl = new IconOptionsControl();

	BView* toolConfigView = new BView("tool config", B_WILL_DRAW);
	toolConfigView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fToolConfigLayout = new BCardLayout();
	toolConfigView->SetLayout(fToolConfigLayout);

	fInspectorView = new InspectorView();
	fInspectorView->SetMenu(fPropertyMenu);
	fInspectorView->SetCommandStack(fDocument->CommandStack());
	fInspectorView->SetSelection(&fSelection);

	ScrollView* inspectorScrollView = new ScrollView(fInspectorView,
		SCROLL_VERTICAL/* | SCROLL_VERTICAL_MAGIC*/,
		"inspector", B_WILL_DRAW | B_FRAME_EVENTS, B_PLAIN_BORDER, 0);

	BGroupView* objectGroup = new BGroupView(B_VERTICAL, 0.0f);
	BLayoutBuilder::Group<>(objectGroup->GroupLayout())
		.Add(objectMenuBar)
		.Add(objectTreeScrollView)
	;
	BGroupView* resourceGroup = new BGroupView(B_VERTICAL, 0.0f);
	BLayoutBuilder::Group<>(resourceGroup->GroupLayout())
		.Add(resourceMenuBar)
		.Add(resourceTreeScrollView)
	;

	BTabView* objectResourceTabView = new BTabView("object resource tabview",
		B_WIDTH_FROM_LABEL);
	objectResourceTabView->AddTab(objectGroup);
	objectResourceTabView->AddTab(resourceGroup);
	objectResourceTabView->SetBorder(B_NO_BORDER);
	objectResourceTabView->TabAt(0)->SetLabel("Objects");
	objectResourceTabView->TabAt(1)->SetLabel("Resources");
//	objectResourceTabView->SetFont(be_bold_font);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.AddSplit(B_HORIZONTAL, 0.0f, 0.15f)
			.AddGroup(B_HORIZONTAL, 0.0f)
				.AddGroup(B_VERTICAL, 0.0f)
					.Add(menuBar)
					.Add(fileIconGroup)
					.AddSplit(B_VERTICAL, 0.0f, 0.10f)
						.AddGroup(B_VERTICAL, 0.0f, 0.20f)
							.Add(new BSeparatorView(B_HORIZONTAL))
							.Add(new NavigatorView(fDocument, fRenderManager))
							.Add(new BSeparatorView(B_HORIZONTAL))
						.End()
						.Add(objectResourceTabView)
						.AddGroup(B_VERTICAL, 0.0f, 0.35f)
							.Add(new BSeparatorView(B_HORIZONTAL))
							.Add(propertyMenuBar)
							.Add(inspectorScrollView)
						.End()
					.End()
				.End()
				.Add(new BSeparatorView(B_VERTICAL))
			.End()
			.AddGroup(B_VERTICAL, 0.0f)
				.Add(toolConfigView)
				.AddGroup(B_HORIZONTAL, 0.0f)
					.Add(fToolIconControl, 0.0f)
					.AddGlue()
					.Add(undoIconGroup, 0.0f)
				.End()
				.Add(new BSeparatorView(B_HORIZONTAL))
				.AddGroup(B_HORIZONTAL, 0.0f)
					.Add(new BSeparatorView(B_VERTICAL))
					.Add(canvasScrollView)
				.End()
			.End()
		.End()
	;

	objectResourceTabView->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));
	objectResourceTabView->SetExplicitMaxSize(BSize(250, B_SIZE_UNSET));
	inspectorScrollView->SetExplicitMinSize(BSize(150, B_SIZE_UNSET));
	inspectorScrollView->SetExplicitMaxSize(BSize(250, B_SIZE_UNSET));

	fView->MakeFocus(true);

	fFileMenu->SetTargetForItems(this);
	fEditMenu->SetTargetForItems(this);
	newWindowMI->SetTarget(be_app);

	_InitTools();

	fView->SetCommandStack(fDocument->CommandStack());

	fDocument->CommandStack()->AddListener(&fCommandStackListener);
	_ObjectChanged(fDocument->CommandStack());

	AddShortcut('Z', B_COMMAND_KEY, new BMessage(MSG_UNDO));
	AddShortcut('Z', B_COMMAND_KEY | B_SHIFT_KEY, new BMessage(MSG_REDO));
}

// destructor
Window::~Window()
{
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

		case MSG_SET_TOOL: {
			int32 index;
			if (message->FindInt32("tool", &index) == B_OK) {
				if (Tool* tool = (Tool*)fTools.ItemAt(index)) {
					fView->SetState(tool->ToolViewState(fView, fDocument,
						&fSelection));
					fToolConfigLayout->SetVisibleItem(index);
				}
			}
			break;
		}

		default:
			BWindow::MessageReceived(message);
	}
}

// QuitRequested
bool
Window::QuitRequested()
{
	BMessage quitMessage(MSG_WINDOW_QUIT);
	quitMessage.AddRect("window frame", Frame());
	be_app->PostMessage(&quitMessage);
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

	// add tool configuration interface
	BView* configView = tool->ConfigView();
	if (configView == NULL) {
		configView = new BView("dummy", 0);
		configView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	}
	fToolConfigLayout->AddView(configView);

	if (count == 0) {
		// this was the first tool
		fView->SetState(tool->ToolViewState(fView, fDocument, &fSelection));
		fToolConfigLayout->SetVisibleItem(0L);
	}
}

// #pragma mark -

// _InitTools
void
Window::_InitTools()
{
	// create canvas tools
//	AddTool(new(std::nothrow) PickTool());
	AddTool(new(std::nothrow) TransformTool());
	AddTool(new(std::nothrow) BrushTool());
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
		fUndoIcon->SetEnabled(fUndoMI->IsEnabled());
		if (fUndoMI->IsEnabled())
			fUndoMI->SetLabel(label.String());
		else
			fUndoMI->SetLabel("<nothing to undo>");

		// relable Redo item and update enabled status
		label.SetTo("Redo");
		fRedoMI->SetEnabled(fDocument->CommandStack()->GetRedoName(label));
		fRedoIcon->SetEnabled(fRedoMI->IsEnabled());
		if (fRedoMI->IsEnabled())
			fRedoMI->SetLabel(label.String());
		else
			fRedoMI->SetLabel("<nothing to redo>");
	}
}
