#include "Window.h"
#include "ui_Window.h"

#include "BrushTool.h"
#include "CanvasView.h"
#include "CommandStack.h"
#include "Document.h"
#include "IconButton.h"
#include "RenderManager.h"
#include "Tool.h"
#include "ToolConfigView.h"


enum {
	MSG_UNDO				= 'undo',
	MSG_REDO				= 'redo',
	MSG_SET_TOOL			= 'sltl'
};


Window::Window(BRect frame, const char* title, Document* document, Layer* layer,
	QWidget* parent)
	:
	BWindow(parent),
	fUi(new Ui::Window),
	fView(new CanvasView()),
	fDocument(document),
	fRenderManager(NULL),
// TODO:...
//	fCommandStackListener(this)
	fCommandStackListener(NULL)
{
	fUi->setupUi(this);

	fRenderManager = new RenderManager(fDocument);
	// TODO: Check error
	fRenderManager->Init();

	fView->Init(fDocument, fRenderManager);
	fUi->canvasScrollView->SetScrollTarget(fView);

	_InitTools();

	fView->SetCommandStack(fDocument->CommandStack());

	fDocument->CommandStack()->AddListener(&fCommandStackListener);
	_ObjectChanged(fDocument->CommandStack());
}


Window::~Window()
{
	delete fUi;
}


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
	fUi->toolIconControl->AddOption(icon);

	// add tool configuration interface
	BView* configView = tool->ConfigView();
	if (configView == NULL) {
		configView = new BView("dummy", 0);
//		configView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	}
	fUi->toolConfigView->addWidget(configView);

	if (count == 0) {
		// this was the first tool
		fView->SetState(tool->ToolViewState(fView, fDocument, &fSelection));
		fUi->toolConfigView->setCurrentIndex(0);
	}
}


status_t
Window::StoreSettings(BMessage& settings) const
{
// TODO:...
	return B_ERROR;
}


void
Window::RestoreSettings(const BMessage& settings)
{
	// TODO:...
}


void
Window::Show()
{
	show();
}


// #pragma mark -


void
Window::_InitTools()
{
	// create canvas tools
//	AddTool(new(std::nothrow) PickTool());
//	AddTool(new(std::nothrow) TransformTool());
	AddTool(new(std::nothrow) BrushTool());
//	AddTool(new(std::nothrow) TextTool());
}


void
Window::_ObjectChanged(const Notifier* object)
{
#if 0
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
#endif
}
