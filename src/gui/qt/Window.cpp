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
#include "TextTool.h"
#include "ToolConfigView.h"
#include "TransformTool.h"


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
	fView(NULL),
	fDocument(document),
	fRenderManager(NULL),
	fCommandStackListener(this)
{
	fUi->setupUi(this);

	fRenderManager = new RenderManager(fDocument);
	// TODO: Check error
	fRenderManager->Init();

	const int iconSize = 16;
	const BRect toolIconBounds(0, 0, 15, 15);
//	float iconGroupInset = 3.0f;

	// Undo/Redo icon group
	fUndoIcon = new IconButton("undo", 0, NULL, new BMessage(MSG_UNDO), this);
	fUndoIcon->SetIcon(301, iconSize);
	fUndoIcon->TrimIcon(toolIconBounds);
	fRedoIcon = new IconButton("redo", 0, NULL, new BMessage(MSG_REDO), this);
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

	QBoxLayout* layout = new QHBoxLayout(fUi->undoIconControl);
	layout->addWidget(fUndoIcon);
	layout->addWidget(fRedoIcon);
	QFrame* line = new QFrame();
	line->setFrameShape(QFrame::VLine);
	layout->addWidget(line);
	layout->addWidget(fConfirmIcon);
	layout->addWidget(fCancelIcon);

	_ReplaceWidget(fUi->navigatorViewDummy,
		new NavigatorView(fDocument, fRenderManager));

	fView = new CanvasView(fDocument, fRenderManager);
	fUi->canvasScrollView->SetScrollTarget(fView);

	fView->MakeFocus(true);

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
						&fSelection, &fCurrentColor));
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
		fView->SetState(tool->ToolViewState(fView, fDocument, &fSelection,
			&fCurrentColor));
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
	AddTool(new(std::nothrow) TransformTool());
	AddTool(new(std::nothrow) BrushTool());
	AddTool(new(std::nothrow) TextTool());
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
		fUndoIcon->SetEnabled(fUi->actionUndo->isEnabled());
		if (fUi->actionUndo->isEnabled())
			fUi->actionUndo->setText(label.ToQString());
		else
			fUi->actionUndo->setText(QString::fromUtf8("<nothing to undo>"));

		// relable Redo item and update enabled status
		label.SetTo("Redo");
		fUi->actionRedo->setEnabled(
			fDocument->CommandStack()->GetRedoName(label));
		fRedoIcon->SetEnabled(fUi->actionRedo->isEnabled());
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
