#include "Window.h"
#include "ui_Window.h"

#include "BrushTool.h"
#include "CanvasView.h"
#include "CommandStack.h"
#include "Document.h"
#include "IconButton.h"
#include "NavigatorView.h"
#include "PickTool.h"
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
	fCommandStackListener(this)
{
	fUi->setupUi(this);

	fRenderManager = new RenderManager(fDocument);
	// TODO: Check error
	fRenderManager->Init();

	_ReplaceWidget(fUi->navigatorViewDummy,
		new NavigatorView(fDocument, fRenderManager));

	fView->Init(fDocument, fRenderManager);
	fUi->canvasScrollView->SetScrollTarget(fView);

	_InitTools();

	fView->SetCommandStack(fDocument->CommandStack());

	fDocument->CommandStack()->AddListener(&fCommandStackListener);
	_ObjectChanged(fDocument->CommandStack());

	connect(fUi->actionUndo, SIGNAL(triggered()), SLOT(_UndoActionInvoked()));
	connect(fUi->actionRedo, SIGNAL(triggered()), SLOT(_RedoActionInvoked()));
}


Window::~Window()
{
	delete fUi;
}


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
					fUi->toolConfigView->setCurrentIndex(index);
				}
			}
			break;
		}

		default:
			BWindow::MessageReceived(message);
	}
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
	AddTool(new(std::nothrow) PickTool());
//	AddTool(new(std::nothrow) TransformTool());
	AddTool(new(std::nothrow) BrushTool());
//	AddTool(new(std::nothrow) TextTool());
}


void
Window::_ObjectChanged(const Notifier* object)
{
	if (!fDocument)
		return;

	if (object == fDocument->CommandStack()) {
		// relable Undo item and update enabled status
		BString label("Undo");
		fUi->actionUndo->setEnabled(
			fDocument->CommandStack()->GetUndoName(label));
//		fUndoIcon->SetEnabled(fUndoMI->IsEnabled());
		if (fUi->actionUndo->isEnabled())
			fUi->actionUndo->setText(label.ToQString());
		else
			fUi->actionUndo->setText(QString::fromUtf8("<nothing to undo>"));

		// relable Redo item and update enabled status
		label.SetTo("Redo");
		fUi->actionRedo->setEnabled(
			fDocument->CommandStack()->GetRedoName(label));
//		fRedoIcon->SetEnabled(fRedoMI->IsEnabled());
		if (fUi->actionRedo->isEnabled())
			fUi->actionRedo->setText(label.ToQString());
		else
			fUi->actionRedo->setText(QString::fromUtf8("<nothing to redo>"));
	}
}


void
Window::_ReplaceWidget(QWidget*& toReplace, QWidget* replacement)
{
	QLayout* layout = toReplace->parentWidget()->layout();
	if (QBoxLayout* boxLayout = dynamic_cast<QBoxLayout*>(layout)) {
		boxLayout->insertWidget(boxLayout->indexOf(toReplace), replacement);
		boxLayout->removeWidget(toReplace);
		delete toReplace;
		toReplace = replacement;
	} else {
		debugger("Window::_ReplaceWidget(): Unsupported layout!");
	}
}


void
Window::_UndoActionInvoked()
{
	fDocument->CommandStack()->Undo();
}


void
Window::_RedoActionInvoked()
{
	fDocument->CommandStack()->Redo();
}
