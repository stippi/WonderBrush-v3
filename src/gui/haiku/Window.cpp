/*
 * Copyright 2007-2013 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "Window.h"

#include <Alert.h>
#include <Application.h>
#include <Bitmap.h>
#include <Box.h>
#include <Catalog.h>
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

#include "BitmapExporter.h"
#include "BitmapSetSaver.h"
#include "BrushTool.h"
#include "CanvasView.h"
#include "CompoundEdit.h"
#include "DefaultColumnTreeModel.h"
#include "Document.h"
#include "DocumentSaver.h"
#include "EditManager.h"
#include "Filter.h"
#include "FilterDropShadow.h"
#include "FilterSaturation.h"
#include "IconButton.h"
#include "IconOptionsControl.h"
#include "InspectorView.h"
//#include "LayerTreeModel.h"
#include "MessageExporter.h"
#include "NativeSaver.h"
#include "NavigatorView.h"
#include "ObjectAddedEdit.h"
#include "ObjectTreeView.h"
#include "PathTool.h"
#include "RectangleTool.h"
#include "ResizeImagePanel.h"
#include "TextTool.h"
#include "ToolConfigView.h"
#include "ToolListener.h"
#include "TransformObjectEdit.h"
#include "TransformTool.h"
#include "RemoveObjectsEdit.h"
#include "RenderManager.h"
#include "ResourceTreeView.h"
#include "ScrollView.h"
#include "SimpleFileSaver.h"
#include "SwatchGroup.h"
#include "WonderBrush.h"

#define B_TRANSLATION_CONTEXT "Window"

enum {
	MSG_UNDO						= 'undo',
	MSG_REDO						= 'redo',
	MSG_CONFIRM						= 'cnfm',
	MSG_CANCEL						= 'cncl',
	MSG_SET_TOOL					= 'sltl',
	MSG_IMAGE_RESIZE				= 'imgr',
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

enum {
	MSG_CONFIRMABLE_EDIT_STARTED	= 'cest',
	MSG_CONFIRMABLE_EDIT_FINISHED	= 'cefn',
};

class Window::ToolListener : public ::ToolListener {
public:
	ToolListener(const BMessenger& messenger)
		: fMessenger(messenger)
	{
	}

	virtual ~ToolListener()
	{
	}

	virtual void ConfirmableEditStarted()
	{
		fMessenger.SendMessage(MSG_CONFIRMABLE_EDIT_STARTED);
	}

	virtual void ConfirmableEditFinished()
	{
		fMessenger.SendMessage(MSG_CONFIRMABLE_EDIT_FINISHED);
	}

private:
	BMessenger fMessenger;
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
	, fCurrentToolIndex(-1)
	, fCurrentTool(NULL)
	, fToolListener(new Window::ToolListener(BMessenger(this)))
	, fMessageAfterSave(NULL)
	, fExporter(NULL)
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
	fFileMenu->AddItem(new BMenuItem("Open" B_UTF8_ELLIPSIS,
		new BMessage(MSG_OPEN), 'O'));
	fFileMenu->AddSeparatorItem();
	fFileMenu->AddItem(new BMenuItem("Save",
		new BMessage(MSG_SAVE), 'S'));
	fFileMenu->AddItem(new BMenuItem("Save as" B_UTF8_ELLIPSIS,
		new BMessage(MSG_SAVE_AS), 'S', B_SHIFT_KEY));
	fFileMenu->AddSeparatorItem();
	fFileMenu->AddItem(new BMenuItem("Export",
		new BMessage(MSG_EXPORT), 'S', B_OPTION_KEY));
	fFileMenu->AddItem(new BMenuItem("Export as" B_UTF8_ELLIPSIS,
		new BMessage(MSG_EXPORT_AS), 'S', B_SHIFT_KEY | B_OPTION_KEY));
	fFileMenu->AddSeparatorItem();
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

	fImageMenu = _CreateImageMenu();
	menuBar->AddItem(fImageMenu);

	fObjectMenu = _CreateObjectMenu();
	menuBar->AddItem(fObjectMenu);

	fResourceMenu = new BMenu("Resource");
	menuBar->AddItem(fResourceMenu);

	fPropertyMenu = new BMenu("Property");
	menuBar->AddItem(fPropertyMenu);

	fLayerTreeView = new ObjectTreeView(fDocument.Get(), &fSelection,
		fEditContext);
//	fLayerTreeView->SetModel(fLayerTreeModel);
	fLayerTreeView->SetModel(new DefaultColumnTreeModel);
	ScrollView* objectTreeScrollView = new ScrollView(fLayerTreeView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL/* | SCROLL_HORIZONTAL_MAGIC
		| SCROLL_VERTICAL_MAGIC*/ | SCROLL_VISIBLE_RECT_IS_CHILD_BOUNDS,
		"layer tree", B_WILL_DRAW | B_FRAME_EVENTS, B_PLAIN_BORDER,
		BORDER_BOTTOM);

	fResourceTreeView = new ResourceTreeView(fDocument.Get(), &fSelection,
		fEditContext);
	fResourceTreeView->SetModel(new DefaultColumnTreeModel);
	ScrollView* resourceTreeScrollView = new ScrollView(fResourceTreeView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL/* | SCROLL_HORIZONTAL_MAGIC
		| SCROLL_VERTICAL_MAGIC*/ | SCROLL_VISIBLE_RECT_IS_CHILD_BOUNDS,
		"layer tree", B_WILL_DRAW | B_FRAME_EVENTS, B_PLAIN_BORDER,
		BORDER_BOTTOM);

	fRenderManager = new RenderManager(fDocument.Get());
	// TODO: Check error
	fRenderManager->Init();
	fView = new CanvasView(fDocument.Get(), fEditContext, fRenderManager);
	ScrollView* canvasScrollView = new ScrollView(fView,
		SCROLL_HORIZONTAL | SCROLL_VERTICAL | SCROLL_NO_FRAME
		| SCROLL_VISIBLE_RECT_IS_CHILD_BOUNDS, "canvas",
		B_WILL_DRAW | B_FRAME_EVENTS, B_NO_BORDER);

	const int iconSize = 16;
	const BRect toolIconBounds(0, 0, 15, 15);
	float iconGroupInset = 3.0f;

	// File icon group
	IconButton* newButton = new IconButton("new", 0, NULL,
		new BMessage(MSG_NEW_DOCUMENT), be_app);
	newButton->SetIcon(201, iconSize);
	newButton->TrimIcon(toolIconBounds);
	newButton->SetToolTip(B_TRANSLATE("New document" B_UTF8_ELLIPSIS));

	IconButton* openButton = new IconButton("open", 0, NULL,
		new BMessage(MSG_OPEN), this);
	openButton->SetIcon(202, iconSize);
	openButton->TrimIcon(toolIconBounds);
	openButton->SetToolTip(B_TRANSLATE("Open document" B_UTF8_ELLIPSIS));

	IconButton* saveButton = new IconButton("save", 0, NULL,
		new BMessage(MSG_SAVE), this);
	saveButton->SetIcon(204, iconSize);
	saveButton->TrimIcon(toolIconBounds);
	saveButton->SetToolTip(B_TRANSLATE("Save document"));

	IconButton* exportButton = new IconButton("export", 0, NULL,
		new BMessage(MSG_EXPORT), this);
	exportButton->SetIcon(203, iconSize);
	exportButton->TrimIcon(toolIconBounds);
	exportButton->SetToolTip(B_TRANSLATE("Export document"));

	IconButton* closeButton = new IconButton("close", 0, NULL,
		new BMessage(B_QUIT_REQUESTED), this);
	closeButton->SetIcon(205, iconSize);
	closeButton->TrimIcon(toolIconBounds);
	closeButton->SetToolTip(B_TRANSLATE("Close document"));

	BGroupView* fileIconGroup = new BGroupView(B_HORIZONTAL, 0.0f);
	BLayoutBuilder::Group<>(fileIconGroup->GroupLayout())
		.Add(newButton)
		.Add(openButton)
		.Add(new BSeparatorView(B_VERTICAL))
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
	fConfirmIcon = new IconButton("confirm", 0, NULL,
		new BMessage(MSG_CONFIRM));
	fConfirmIcon->SetIcon(303, iconSize);
	fConfirmIcon->TrimIcon(toolIconBounds);
	fCancelIcon = new IconButton("cancel", 0, NULL,
		new BMessage(MSG_CANCEL));
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

	fInspectorView = new InspectorView(fEditContext);
	fInspectorView->SetMenu(fPropertyMenu);
	fInspectorView->SetEditManager(fDocument->EditManager());
	fInspectorView->SetSelection(&fSelection);

	ScrollView* inspectorScrollView = new ScrollView(fInspectorView,
		SCROLL_VERTICAL/* | SCROLL_VERTICAL_MAGIC*/,
		"inspector", B_WILL_DRAW | B_FRAME_EVENTS, B_PLAIN_BORDER, 0);

	BGroupView* objectGroup = new BGroupView(B_VERTICAL, 0.0f);
	BLayoutBuilder::Group<>(objectGroup->GroupLayout())
//		.Add(objectMenuBar)
		.Add(objectTreeScrollView)
	;
	BGroupView* resourceGroup = new BGroupView(B_VERTICAL, 0.0f);
	BLayoutBuilder::Group<>(resourceGroup->GroupLayout())
//		.Add(resourceMenuBar)
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
		.Add(menuBar)
		.AddSplit(fHorizontalSplit, 0.15f)
			.AddGroup(B_HORIZONTAL, 0.0f)
				.AddGroup(B_VERTICAL, 0.0f)
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
//							.Add(propertyMenuBar)
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

	delete fMessageAfterSave;
	delete fExporter;
}

// MessageReceived
void
Window::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_OPEN:
			// If our document is empty, we want the file to open in this
			// window.
			if (fDocument->IsEmpty()) {
				message->AddPointer("document", fDocument.Get());
			}
			be_app->PostMessage(message);
			break;

		case MSG_SAVE:
		case MSG_EXPORT:
		{
			DocumentSaver* saver;
			if (message->what == MSG_SAVE)
				saver = fDocument->NativeSaver();
			else
				saver = fDocument->ExportSaver();
			if (saver != NULL) {
				saver->Save(fDocument);
				_PickUpActionBeforeSave();
				break;
			} // else fall through
		}
		case MSG_SAVE_AS:
		case MSG_EXPORT_AS:
		{
			int32 exportMode;
			if (message->FindInt32("export mode", &exportMode) < B_OK)
				exportMode = EXPORT_MODE_MESSAGE;
			entry_ref ref;
			const char* name;
			if (message->FindRef("directory", &ref) == B_OK
				&& message->FindString("name", &name) == B_OK) {
				// this message comes from the file panel
				BDirectory dir(&ref);
				BEntry entry;
				if (dir.InitCheck() >= B_OK
					&& entry.SetTo(&dir, name, true) >= B_OK
					&& entry.GetRef(&ref) >= B_OK) {

					// create the document saver and remember it for later
					DocumentSaver* saver = _CreateSaver(ref, exportMode);
					if (saver != NULL) {
						if (fDocument->WriteLock()) {
							if (exportMode == EXPORT_MODE_MESSAGE)
								fDocument->SetNativeSaver(saver);
							else
								fDocument->SetExportSaver(saver);
							_UpdateWindowTitle();
							fDocument->WriteUnlock();
						}
						saver->Save(fDocument);
						_PickUpActionBeforeSave();
					}
				}
// TODO: ...
//				_SyncPanels(fSavePanel, fOpenPanel);
			} else {
				// configure the file panel
				uint32 requestRefWhat = MSG_SAVE_AS;
				bool isExportMode = message->what == MSG_EXPORT_AS
					|| message->what == MSG_EXPORT;
				if (isExportMode)
					requestRefWhat = MSG_EXPORT_AS;
				const char* saveText = _FileName(isExportMode);

				BMessage requestRef(requestRefWhat);
				if (saveText != NULL)
					requestRef.AddString("save text", saveText);
				requestRef.AddMessenger("target", BMessenger(this, this));
				be_app->PostMessage(&requestRef);
			}
			break;
		}
		case B_CANCEL:
			// Forget what was to be done after the file panel was used.
			delete fMessageAfterSave;
			fMessageAfterSave = NULL;
			break;

		case B_SIMPLE_DATA:
			be_app->PostMessage(message);
			break;

		case MSG_UNDO:
			fDocument->EditManager()->Undo(fEditContext);
			break;
		case MSG_REDO:
			fDocument->EditManager()->Redo(fEditContext);
			break;

		case MSG_CONFIRM:
			if (fCurrentTool != NULL)
				fCurrentTool->Confirm();
			break;
		case MSG_CANCEL:
			if (fCurrentTool != NULL)
				fCurrentTool->Cancel();
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
		
		case MSG_IMAGE_RESIZE:
			_RunResizeImageDialog();
			break;

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

		case MSG_CONFIRMABLE_EDIT_STARTED:
			fConfirmIcon->SetEnabled(true);
			fCancelIcon->SetEnabled(true);
			break;

		case MSG_CONFIRMABLE_EDIT_FINISHED:
			fConfirmIcon->SetEnabled(false);
			fCancelIcon->SetEnabled(false);
			break;

		default:
			BWindow::MessageReceived(message);
	}
}

// QuitRequested
bool
Window::QuitRequested()
{
	if (!_CheckSaveDocument(CurrentMessage()))
		return false;

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
	if (fCurrentTool != NULL) {
		fCurrentTool->Confirm();
		fCurrentTool->RemoveListener(fToolListener);
	}

	fCurrentTool = (Tool*)fTools.ItemAt(index);

	if (fCurrentTool != NULL) {
		fCurrentTool->AddListener(fToolListener);
		fView->SetState(fCurrentTool->ToolViewState(fView, fDocument.Get(),
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

// _CreateImageMenu
BMenu*
Window::_CreateImageMenu() const
{
	BMenu* menu = new BMenu("Image");

	BMenuItem* item = new BMenuItem("Resize" B_UTF8_ELLIPSIS,
		new BMessage(MSG_IMAGE_RESIZE));
	menu->AddItem(item);

	return menu;
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

// _RunResizeImageDialog
void
Window::_RunResizeImageDialog()
{
	ResizeImagePanel* panel = new ResizeImagePanel(this,
		BRect(0, 0, 250, 200));
	panel->Show();
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
		new(std::nothrow) ObjectAddedEdit(object, &fSelection),
		fEditContext);
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
		new(std::nothrow) RemoveObjectsEdit(list, &fSelection),
		fEditContext);
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

	fDocument->EditManager()->Perform(UndoableEditRef(compoundEdit, true),
		fEditContext);
}

// #pragma mark -

// _CheckSaveDocument
bool
Window::_CheckSaveDocument(const BMessage* currentMessage)
{
	if (fDocument->IsEmpty() || fDocument->EditManager()->IsSaved())
		return true;

	// Make sure the user sees us.
	Activate();

	BAlert* alert = new BAlert("save", 
		B_TRANSLATE("Save changes before closing?"),
			B_TRANSLATE("Cancel"), B_TRANSLATE("Don't save"),
			B_TRANSLATE("Save"), B_WIDTH_AS_USUAL, B_OFFSET_SPACING,
			B_WARNING_ALERT);
	alert->SetShortcut(0, B_ESCAPE);
	alert->SetShortcut(1, 'd');
	alert->SetShortcut(2, 's');
	int32 choice = alert->Go();
	switch (choice) {
		case 0:
			// cancel
			return false;
		case 1:
			// don't save
			return true;
		case 2:
		default:
			// cancel (save first) but pick up what we were doing before
			PostMessage(MSG_SAVE);
			if (currentMessage != NULL) {
				delete fMessageAfterSave;
				fMessageAfterSave = new BMessage(*currentMessage);
			}
			return false;
	}
}

// _PickUpActionBeforeSave
void
Window::_PickUpActionBeforeSave()
{
	if (fDocument->WriteLock()) {
		fDocument->EditManager()->Save();
		fDocument->WriteUnlock();
	}

	if (fMessageAfterSave == NULL)
		return;

	PostMessage(fMessageAfterSave);
	delete fMessageAfterSave;
	fMessageAfterSave = NULL;
}


// _Save
void
Window::_Save()
{
	if (fExporter == NULL)
		fExporter = new(std::nothrow) MessageExporter();

	if (fExporter == NULL)
		return;

	_Save(fExporter);
}

// _Save
void
Window::_Save(Exporter* exporter) const
{
	entry_ref ref;
	get_ref_for_path("/boot/home/Desktop/export_test.wbi", &ref);
	
	exporter->Export(fDocument.Get(), ref);
}

// _CreateSaver
DocumentSaver*
Window::_CreateSaver(const entry_ref& ref, uint32 exportMode) const
{
	DocumentSaver* saver;

	switch (exportMode) {
//		case EXPORT_MODE_FLAT_ICON:
//			saver = new SimpleFileSaver(new FlatIconExporter(), ref);
//			break;
//
//		case EXPORT_MODE_ICON_ATTR:
//		case EXPORT_MODE_ICON_MIME_ATTR: {
//			const char* attrName
//				= exportMode == EXPORT_MODE_ICON_ATTR ?
//					kVectorAttrNodeName : kVectorAttrMimeName;
//			saver = new AttributeSaver(ref, attrName);
//			break;
//		}

//		case EXPORT_MODE_ICON_RDEF:
//			saver = new SimpleFileSaver(new RDefExporter(), ref);
//			break;
//		case EXPORT_MODE_ICON_SOURCE:
//			saver = new SimpleFileSaver(new SourceExporter(), ref);
//			break;

		case EXPORT_MODE_BITMAP:
			saver = new SimpleFileSaver(new BitmapExporter(
				fDocument->Bounds()), ref);
			break;

		case EXPORT_MODE_BITMAP_SET:
			saver = new BitmapSetSaver(ref);
			break;

//		case EXPORT_MODE_SVG:
//			saver = new SimpleFileSaver(new SVGExporter(), ref);
//			break;

		case EXPORT_MODE_MESSAGE:
		default:
			saver = new NativeSaver(ref);
			break;
	}

	return saver;
}

// _FileName
const char*
Window::_FileName(bool preferExporter) const
{
	FileSaver* saver1;
	FileSaver* saver2;
	if (preferExporter) {
		saver1 = dynamic_cast<FileSaver*>(fDocument->ExportSaver());
		saver2 = dynamic_cast<FileSaver*>(fDocument->NativeSaver());
	} else {
		saver1 = dynamic_cast<FileSaver*>(fDocument->NativeSaver());
		saver2 = dynamic_cast<FileSaver*>(fDocument->ExportSaver());
	}
	const char* fileName = NULL;
	if (saver1 != NULL)
		fileName = saver1->Ref()->name;
	if ((fileName == NULL || fileName[0] == '\0') && saver2 != NULL)
		fileName = saver2->Ref()->name;
	return fileName;
}

// _UpdateWindowTitle
void
Window::_UpdateWindowTitle()
{
	const char* fileName = _FileName(false);
	if (fileName != NULL)
		SetTitle(fileName);
	else
		SetTitle(B_TRANSLATE_SYSTEM_NAME("WonderBrush"));
}

