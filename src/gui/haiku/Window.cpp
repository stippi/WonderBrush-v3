/*
 * Copyright 2007-2013 Stephan Aßmus <superstippi@gmx.de>
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
#include "CompoundEdit.h"
#include "DefaultColumnTreeModel.h"
#include "Document.h"
#include "EditManager.h"
#include "Filter.h"
#include "FilterDropShadow.h"
#include "FilterSaturation.h"
#include "IconButton.h"
#include "IconOptionsControl.h"
#include "InspectorView.h"
//#include "LayerTreeModel.h"
#include "NavigatorView.h"
#include "ObjectAddedEdit.h"
#include "ObjectTreeView.h"
#include "PathTool.h"
#include "RectangleTool.h"
#include "TextTool.h"
#include "ToolConfigView.h"
#include "TransformObjectEdit.h"
#include "TransformTool.h"
#include "RemoveObjectsEdit.h"
#include "RenderManager.h"
#include "ResourceTreeView.h"
#include "ScrollView.h"
#include "SwatchGroup.h"
#include "WonderBrush.h"

enum {
	MSG_UNDO						= 'undo',
	MSG_REDO						= 'redo',
	MSG_SET_TOOL					= 'sltl',
	MSG_ADD_LAYER					= 'addl',
	MSG_ADD_FILTER					= 'addf',
	MSG_RESET_TRANSFORMATION		= 'rttr',
	MSG_REMOVE						= 'rmvo',
};

enum {
	FILTER_DROP_SHADOW				= 'drps',
	FILTER_GAUSSIAN_BLUR			= 'gblr',
	FILTER_SATURATION				= 'srtn',
};

class Window::SelectionListener : public Selection::Listener {
public:
	SelectionListener(BMenuItem* removeItem, Selection* selection)
		: fRemoveMI(removeItem)
		, fSelection(selection)
	{
	}

	virtual ~SelectionListener()
	{
	}

	virtual	void ObjectSelected(const Selectable& object,
		const Selection::Controller* controller)
	{
		fRemoveMI->SetEnabled(_CountRemovableObjects() > 0);
	}

	virtual	void ObjectDeselected(const Selectable& object,
		const Selection::Controller* controller)
	{
		fRemoveMI->SetEnabled(_CountRemovableObjects() > 0);
	}

private:

	int32 _CountRemovableObjects() const
	{
		int32 objectCount = 0;
		for (int32 i = fSelection->CountSelected() - 1; i >= 0; i--) {
			const Selectable& selectable = fSelection->SelectableAt(i);
			Object* object = dynamic_cast<Object*>(selectable.Get());
			if (object != NULL && object->Parent() != NULL)
				objectCount++;
		}

		return objectCount;
	}

private:
	BMenuItem*	fRemoveMI;
	Selection*	fSelection;
};

// #pragma mark -

// constructor
Window::Window(BRect frame, const char* title, Document* document,
			Layer* layer)
	: BWindow(frame, title, B_DOCUMENT_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS)
	, fDocument(document)
	, fRenderManager(NULL)
	, fEditManagerListener(this)
//	, fLayerTreeModel(new LayerTreeModel(fDocument))
{
	// TODO: fix for when document == NULL

	BMenuBar* menuBar = new BMenuBar("main menu");
	fFileMenu = new BMenu("File");
	menuBar->AddItem(fFileMenu);
	BMenuItem* newWindowMI = new BMenuItem("New window",
		new BMessage(MSG_NEW_WINDOW), 'N', B_SHIFT_KEY);
	fFileMenu->AddItem(newWindowMI);
	BMenuItem* newDocumentMI = new BMenuItem("New document",
		new BMessage(MSG_NEW_DOCUMENT), 'N');
	fFileMenu->AddItem(newDocumentMI);
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

	fRemoveMI = new BMenuItem("Remove",
		new BMessage(MSG_REMOVE));
	fRemoveMI->SetEnabled(false);

	BMenuBar* objectMenuBar = new BMenuBar("object menu");
	fObjectMenu = _CreateObjectMenu();
	objectMenuBar->AddItem(fObjectMenu);

	BMenuBar* resourceMenuBar = new BMenuBar("resource menu");
	fResourceMenu = new BMenu("Resource");
	resourceMenuBar->AddItem(fResourceMenu);

	BMenuBar* propertyMenuBar = new BMenuBar("property menu");
	fPropertyMenu = new BMenu("Property");
	propertyMenuBar->AddItem(fPropertyMenu);

	fLayerTreeView = new ObjectTreeView(fDocument.Get(), &fSelection);
//	fLayerTreeView->SetModel(fLayerTreeModel);
	fLayerTreeView->SetModel(new DefaultColumnTreeModel);
	ScrollView* objectTreeScrollView = new ScrollView(fLayerTreeView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL/* | SCROLL_HORIZONTAL_MAGIC
		| SCROLL_VERTICAL_MAGIC*/ | SCROLL_VISIBLE_RECT_IS_CHILD_BOUNDS,
		"layer tree", B_WILL_DRAW | B_FRAME_EVENTS, B_PLAIN_BORDER,
		BORDER_BOTTOM);

	fResourceTreeView = new ResourceTreeView(fDocument.Get(), &fSelection);
	fResourceTreeView->SetModel(new DefaultColumnTreeModel);
	ScrollView* resourceTreeScrollView = new ScrollView(fResourceTreeView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL/* | SCROLL_HORIZONTAL_MAGIC
		| SCROLL_VERTICAL_MAGIC*/ | SCROLL_VISIBLE_RECT_IS_CHILD_BOUNDS,
		"layer tree", B_WILL_DRAW | B_FRAME_EVENTS, B_PLAIN_BORDER,
		BORDER_BOTTOM);

	fRenderManager = new RenderManager(fDocument.Get());
	// TODO: Check error
	fRenderManager->Init();
	fView = new CanvasView(fDocument.Get(), fRenderManager);
	ScrollView* canvasScrollView = new ScrollView(fView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL | SCROLL_NO_FRAME
		| SCROLL_VISIBLE_RECT_IS_CHILD_BOUNDS, "canvas",
		B_WILL_DRAW | B_FRAME_EVENTS, B_NO_BORDER);

	const int iconSize = 16;
	const BRect toolIconBounds(0, 0, 15, 15);
	float iconGroupInset = 3.0f;

	// File icon group
	IconButton* newButton = new IconButton("new", 0, NULL,
		new BMessage(MSG_NEW_WINDOW), be_app);
	newButton->SetIcon(201, iconSize);
	newButton->TrimIcon(toolIconBounds);
	IconButton* openButton = new IconButton("open", 0);
	openButton->SetIcon(202, iconSize);
	openButton->TrimIcon(toolIconBounds);
openButton->SetEnabled(false);
	IconButton* saveButton = new IconButton("save", 0);
	saveButton->SetIcon(204, iconSize);
	saveButton->TrimIcon(toolIconBounds);
saveButton->SetEnabled(false);
	IconButton* exportButton = new IconButton("export", 0);
	exportButton->SetIcon(203, iconSize);
	exportButton->TrimIcon(toolIconBounds);
exportButton->SetEnabled(false);
	IconButton* closeButton = new IconButton("close", 0, NULL,
		new BMessage(B_QUIT_REQUESTED), this);
	closeButton->SetIcon(205, iconSize);
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
		.AddGlue()
		.SetInsets(iconGroupInset, iconGroupInset, iconGroupInset,
			iconGroupInset)
	;

	// Zoom icon group
	IconButton* zoomInButton = new IconButton("zoom in", 0, NULL,
		new BMessage(MSG_ZOOM_IN), fView);
	zoomInButton->SetIcon(401, iconSize);
	zoomInButton->TrimIcon(toolIconBounds);
	IconButton* zoomOutButton = new IconButton("zoom out", 0, NULL,
		new BMessage(MSG_ZOOM_OUT), fView);
	zoomOutButton->SetIcon(402, iconSize);
	zoomOutButton->TrimIcon(toolIconBounds);
	IconButton* zoomOriginalButton = new IconButton("zoom original", 0, NULL,
		new BMessage(MSG_ZOOM_ORIGINAL), fView);
	zoomOriginalButton->SetIcon(403, iconSize);
	zoomOriginalButton->TrimIcon(toolIconBounds);
	IconButton* zoomToFit = new IconButton("zoom to fit", 0, NULL,
		new BMessage(MSG_ZOOM_TO_FIT), fView);
	zoomToFit->SetIcon(404, iconSize);
	zoomToFit->TrimIcon(toolIconBounds);
	BGroupView* zoomIconGroup = new BGroupView(B_HORIZONTAL, 0.0f);
	BLayoutBuilder::Group<>(zoomIconGroup->GroupLayout())
		.AddGlue()
		.Add(zoomInButton)
		.Add(zoomOutButton)
		.Add(zoomOriginalButton)
		.Add(zoomToFit)
		.SetInsets(iconGroupInset, iconGroupInset, iconGroupInset,
			iconGroupInset)
	;

	// Undo/Redo icon group
	fUndoIcon = new IconButton("undo", 0, NULL, new BMessage(MSG_UNDO));
	fUndoIcon->SetIcon(301, iconSize);
	fUndoIcon->TrimIcon(toolIconBounds);
	fRedoIcon = new IconButton("redo", 0, NULL, new BMessage(MSG_REDO));
	fRedoIcon->SetIcon(302, iconSize);
	fRedoIcon->TrimIcon(toolIconBounds);
	fConfirmIcon = new IconButton("confirm", 0);
	fConfirmIcon->SetIcon(303, iconSize);
	fConfirmIcon->TrimIcon(toolIconBounds);
	fCancelIcon = new IconButton("cancel", 0);
	fCancelIcon->SetIcon(304, iconSize);
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
		.SetInsets(iconGroupInset, iconGroupInset, iconGroupInset,
			iconGroupInset)
	;

	fToolIconControl = new IconOptionsControl();

	BView* toolConfigView = new BView("tool config", B_WILL_DRAW);
	toolConfigView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	fToolConfigLayout = new BCardLayout();
	toolConfigView->SetLayout(fToolConfigLayout);

	fSwatchGroup = new SwatchGroup("swatch group");
	fSwatchGroup->SetCurrentColor(&fCurrentColor);

	fInspectorView = new InspectorView();
	fInspectorView->SetMenu(fPropertyMenu);
	fInspectorView->SetEditManager(fDocument->EditManager());
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
	objectResourceTabView->SetFont(be_bold_font);

	fHorizontalSplit = new BSplitView(B_HORIZONTAL, 0.0f);
	fVerticalSplit = new BSplitView(B_VERTICAL, 0.0f);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.AddSplit(fHorizontalSplit, 0.15f)
			.AddGroup(B_HORIZONTAL, 0.0f)
				.AddGroup(B_VERTICAL, 0.0f)
					.Add(menuBar)
					.Add(fileIconGroup)
					.AddSplit(fVerticalSplit, 0.10f)
						.AddGroup(B_VERTICAL, 0.0f, 0.30f)
							.Add(new BSeparatorView(B_HORIZONTAL))
							.Add(new NavigatorView(fDocument.Get(),
								fRenderManager))
							.Add(new BSeparatorView(B_HORIZONTAL))
							.Add(zoomIconGroup)
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
				.AddGroup(B_HORIZONTAL, 0.0f)
					.AddGroup(B_VERTICAL, 0.0f)
						.Add(toolConfigView)
						.AddGroup(B_HORIZONTAL, 0.0f)
							.Add(fToolIconControl, 0.0f)
							.AddGlue()
							.Add(undoIconGroup, 0.0f)
						.End()
					.End()
					.Add(new BSeparatorView(B_VERTICAL))
					.Add(fSwatchGroup)
				.End()
				.Add(new BSeparatorView(B_HORIZONTAL))
				.AddGroup(B_HORIZONTAL, 0.0f)
					.Add(new BSeparatorView(B_VERTICAL))
					.Add(canvasScrollView)
				.End()
			.End()
		.End()
	;

	objectResourceTabView->SetExplicitMinSize(BSize(170, B_SIZE_UNSET));
	objectResourceTabView->SetExplicitMaxSize(BSize(250, B_SIZE_UNSET));

	fView->MakeFocus(true);

	fFileMenu->SetTargetForItems(this);
	fEditMenu->SetTargetForItems(this);
	newWindowMI->SetTarget(be_app);
	newDocumentMI->SetTarget(be_app);
	zoomInButton->SetTarget(fView);
	zoomOutButton->SetTarget(fView);
	zoomOriginalButton->SetTarget(fView);
	zoomToFit->SetTarget(fView);
	fRemoveMI->SetTarget(this);

	_InitTools();

	fView->SetEditManager(fDocument->EditManager());

	fDocument->EditManager()->AddListener(&fEditManagerListener);
	_ObjectChanged(fDocument->EditManager());

	fSelectionListener = new(std::nothrow)
		SelectionListener(fRemoveMI, &fSelection);

	fSelection.AddListener(fSelectionListener);

	AddShortcut('Z', B_COMMAND_KEY, new BMessage(MSG_UNDO));
	AddShortcut('Z', B_COMMAND_KEY | B_SHIFT_KEY, new BMessage(MSG_REDO));
}

// destructor
Window::~Window()
{
	fSelection.RemoveListener(fSelectionListener);
	delete fSelectionListener;

	fDocument->EditManager()->RemoveListener(&fEditManagerListener);
	delete fRenderManager;
//	delete fLayerTreeModel;

	fSwatchGroup->SetCurrentColor(NULL);

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
			fDocument->EditManager()->Undo();
			break;
		case MSG_REDO:
			fDocument->EditManager()->Redo();
			break;

		case MSG_OBJECT_CHANGED:
		{
			Notifier* notifier;
			if (message->FindPointer("object", (void**)&notifier) == B_OK)
				_ObjectChanged(notifier);
			break;
		}

		case MSG_SET_TOOL:
		{
			int32 index;
			if (message->FindInt32("tool", &index) == B_OK)
				_SetTool(index);
			break;
		}

		case MSG_ADD_LAYER:
			_AddLayer();
			break;

		case MSG_ADD_FILTER:
		{
			int32 filterID;
			if (message->FindInt32("filter id", &filterID) == B_OK)
				_AddFilter(filterID);
			break;
		}

		case MSG_RESET_TRANSFORMATION:
			_ResetTransformation();
			break;

		case MSG_REMOVE:
			_RemoveObjects();
			break;

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

	BMessage settings;
	StoreSettings(settings);
	quitMessage.AddMessage("window settings", &settings);

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
		_SetTool(0);
	}
}

// store_split_weights
status_t
store_split_weights(BMessage& message, const char* name, BSplitView* view)
{
	BString collapsedName(name);
	collapsedName.Append("_collapsed");

	message.RemoveName(name);
	message.RemoveName(collapsedName.String());

	int32 count = view->GetLayout()->CountItems();
	for (int32 i = 0; i < count; i++) {
		status_t ret = message.AddFloat(name, view->ItemWeight(i));
		if (ret != B_OK)
			return ret;
		ret = message.AddBool(collapsedName.String(),
			view->IsItemCollapsed(i));
		if (ret != B_OK)
			return ret;
	}
	return B_OK;
}

// restore_split_weights
void
restore_split_weights(const BMessage& message, const char* name,
	BSplitView* view)
{
	BString collapsedName(name);
	collapsedName.Append("_collapsed");

	float weight;
	for (int32 i = 0; message.FindFloat(name, i, &weight) == B_OK; i++) {
		if (i >= view->GetLayout()->CountItems())
			break;

		view->SetItemWeight(i, weight, false);

		bool collapsed;
		if (message.FindBool(collapsedName.String(), i, &collapsed) == B_OK)
			view->SetItemCollapsed(i, collapsed);
	}
	view->InvalidateLayout(true);
}

// StoreSettings
status_t
Window::StoreSettings(BMessage& settings) const
{
	status_t ret = B_OK;
	if (ret == B_OK) {
		ret = store_split_weights(settings, "horizontal split",
			fHorizontalSplit);
	}
	if (ret == B_OK) {
		ret = store_split_weights(settings, "vertical split",
			fVerticalSplit);
	}
	if (ret == B_OK) {
		settings.RemoveName("current tool index");
		ret = settings.AddInt32("current tool index", fCurrentToolIndex);
	}
	return ret;
}

// RestoreSettings
void
Window::RestoreSettings(const BMessage& settings)
{
	restore_split_weights(settings, "horizontal split", fHorizontalSplit);
	restore_split_weights(settings, "vertical split", fVerticalSplit);

	int32 toolIndex;
	if (settings.FindInt32("current tool index", &toolIndex) == B_OK)
		_SetTool(toolIndex);
}

// #pragma mark -

// _InitTools
void
Window::_InitTools()
{
	// create canvas tools
	AddTool(new(std::nothrow) TransformTool());
	AddTool(new(std::nothrow) BrushTool());
	AddTool(new(std::nothrow) TextTool());
	AddTool(new(std::nothrow) PathTool());
	AddTool(new(std::nothrow) RectangleTool());
}

// _SetTool
void
Window::_SetTool(int32 index)
{
	if (Tool* tool = (Tool*)fTools.ItemAt(index)) {
		fView->SetState(tool->ToolViewState(fView, fDocument.Get(),
			&fSelection, &fCurrentColor));
		fToolConfigLayout->SetVisibleItem(index);
		fToolIconControl->SetValue(index);

		fCurrentToolIndex = index;
	}
}

// _ObjectChanged
void
Window::_ObjectChanged(const Notifier* object)
{
	if (fDocument == NULL)
		return;

	if (object == fDocument->EditManager()) {
		// relabel Undo item/icon and update enabled status
		BString label("Undo");
		bool enabled = fDocument->EditManager()->GetUndoName(label);
		if (!enabled)
			label = "<nothing to undo>";

		fUndoMI->SetEnabled(enabled);
		fUndoIcon->SetEnabled(enabled);

		fUndoMI->SetLabel(label.String());
		fUndoIcon->SetToolTip(label.String());

		// relabel Redo item/icon and update enabled status
		label.SetTo("Redo");
		enabled = fDocument->EditManager()->GetRedoName(label);
		if (!enabled)
			label = "<nothing to redo>";

		fRedoMI->SetEnabled(enabled);
		fRedoIcon->SetEnabled(enabled);

		fRedoMI->SetLabel(label.String());
		fRedoIcon->SetToolTip(label.String());
	}
}

// _AddFilterMenuItem
void
Window::_AddFilterMenuItem(BMenu* menu, const char* label,
	int32 filterID) const
{
	BMessage* message = new BMessage(MSG_ADD_FILTER);
	message->AddInt32("filter id", filterID);

	menu->AddItem(new BMenuItem(label, message));
}

// _CreateObjectMenu
BMenu*
Window::_CreateObjectMenu() const
{
	BMenu* menu = new BMenu("Object");

	BMenuItem* item = new BMenuItem("Add layer",
		new BMessage(MSG_ADD_LAYER));
	menu->AddItem(item);

	BMenu* filterMenu = new BMenu("Add filter");

	_AddFilterMenuItem(filterMenu, "Drop shadow", FILTER_DROP_SHADOW);
	_AddFilterMenuItem(filterMenu, "Gaussian blur", FILTER_GAUSSIAN_BLUR);
	_AddFilterMenuItem(filterMenu, "Saturation", FILTER_SATURATION);

	menu->AddItem(filterMenu);

	menu->AddItem(new BSeparatorItem());

	item = new BMenuItem("Reset transformation",
		new BMessage(MSG_RESET_TRANSFORMATION));
	menu->AddItem(item);

	menu->AddItem(new BSeparatorItem());

	menu->AddItem(fRemoveMI);

	return menu;
}

// _GetInsertionPosition
bool
Window::_GetInsertionPosition(Layer** _layer, int32* _index) const
{
	// Initial parent layer and insertion index, in case there is no selection
	Layer* parentLayer = fDocument->RootLayer();
	if (parentLayer == NULL) {
		fprintf(stderr, "Window::_GetInsertionPosition(): No root layer.\n");
		return false;
	}

	int32 insertIndex = parentLayer->CountObjects();

	if (!fSelection.IsEmpty()) {
		// Find the last Selectable that represents an Object
		Object* selected = NULL;
		int32 selectionCount = fSelection.CountSelected();
		for (int32 i = 0; i < selectionCount; i++) {
			const Selectable& selectable = fSelection.SelectableAt(i);
			Object* object = dynamic_cast<Object*>(selectable.Get());
			if (object != NULL)
				selected = object;
		}

		if (selected != NULL) {
			parentLayer = selected->Parent();
			insertIndex = parentLayer->IndexOf(selected) + 1;
		}
	}

	*_layer = parentLayer;
	*_index = insertIndex;
	return true;
}

// _AddLayer
void
Window::_AddLayer()
{
	if (fDocument == NULL) {
		fprintf(stderr, "Window::_AddLayer(): No document.\n");
		return;
	}

	AutoWriteLocker _(fDocument.Get());

	Layer* parentLayer;
	int32 insertIndex;
	if (!_GetInsertionPosition(&parentLayer, &insertIndex)) {
		fprintf(stderr, "Window::_AddLayer(): No insert position info.\n");
		return;
	}

	Layer* newLayer = new(std::nothrow) Layer(parentLayer->Bounds());

	_AddObject(parentLayer, insertIndex, newLayer);
}

// _AddFilter
void
Window::_AddFilter(int32 filterID)
{
	if (fDocument == NULL) {
		fprintf(stderr, "Window::_AddLayer(): No document.\n");
		return;
	}

	AutoWriteLocker _(fDocument.Get());

	Layer* parentLayer;
	int32 insertIndex;
	if (!_GetInsertionPosition(&parentLayer, &insertIndex)) {
		fprintf(stderr, "Window::_AddFilter(): No insert position info.\n");
		return;
	}

	Object* filter = NULL;
	switch (filterID) {
		case FILTER_DROP_SHADOW:
			filter = new(std::nothrow) FilterDropShadow();
			break;
		case FILTER_GAUSSIAN_BLUR:
			filter = new(std::nothrow) Filter();
			break;
		case FILTER_SATURATION:
			filter = new(std::nothrow) FilterSaturation();
			break;
	};

	_AddObject(parentLayer, insertIndex, filter);
}

// _AddObject
void
Window::_AddObject(Layer* parentLayer, int32 insertIndex, Object* object)
{
	if (object == NULL || !parentLayer->AddObject(object, insertIndex)) {
		fprintf(stderr,
			"Window::_AddObject(): Failed to allocate or insert new object.\n");
		delete object;
		return;
	}

	fLayerTreeView->DeselectAll();
	fDocument->EditManager()->Perform(
		new(std::nothrow) ObjectAddedEdit(object, &fSelection));
}

// _RemoveObjects
void
Window::_RemoveObjects()
{
	if (fDocument == NULL) {
		fprintf(stderr, "Window::_RemoveObjects(): No document.\n");
		return;
	}

	AutoWriteLocker _(fDocument.Get());

	if (fSelection.IsEmpty())
		return;

	RemoveObjectsEdit::ObjectList list;

	int32 selectionCount = fSelection.CountSelected();
	for (int32 i = 0; i < selectionCount; i++) {
		const Selectable& selectable = fSelection.SelectableAt(i);
		Object* object = dynamic_cast<Object*>(selectable.Get());
		if (object == NULL || object->Parent() == NULL)
			continue;
		list.Add(Reference<Object>(object));
	}

	fDocument->EditManager()->Perform(
		new(std::nothrow) RemoveObjectsEdit(list, &fSelection));
}

// _ResetTransformation
void
Window::_ResetTransformation()
{
	if (fDocument == NULL) {
		fprintf(stderr, "Window::_ResetTransformation(): No document.\n");
		return;
	}

	AutoWriteLocker _(fDocument.Get());

	if (fSelection.IsEmpty()) {
		fprintf(stderr, "Window::_ResetTransformation(): No selection.\n");
		return;
	}

	CompoundEdit* compoundEdit = new(std::nothrow) CompoundEdit(
		"Reset transformation");

	if (compoundEdit == NULL) {
		fprintf(stderr, "Window::_ResetTransformation(): No memory.\n");
		return;
	}

	Transformable identityTransform;

	int32 selectionCount = fSelection.CountSelected();
	for (int32 i = 0; i < selectionCount; i++) {
		const Selectable& selectable = fSelection.SelectableAt(i);
		Object* object = dynamic_cast<Object*>(selectable.Get());
		if (object == NULL)
			continue;

		TransformObjectEdit* edit = new(std::nothrow) TransformObjectEdit(
			object, identityTransform);

		if (edit == NULL
			|| !compoundEdit->AppendEdit(UndoableEditRef(edit, true))) {
			fprintf(stderr, "Window::_ResetTransformation(): No memory.\n");
			return;
		}
	}

	fDocument->EditManager()->Perform(UndoableEditRef(compoundEdit, true));
}
