#include <Application.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Message.h>
#include <String.h>

#include "Window.h"

#include "App.h"
#include "CommandStack.h"
#include "Document.h"
#include "RenderManager.h"
#include "PickToolState.h"
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

	fRenderManager = new RenderManager(fDocument, frame.OffsetToCopy(B_ORIGIN));
	fView = new View(frame, fDocument, fRenderManager);
	AddChild(fView);
	fView->MakeFocus(true);

	fView->SetState(new PickToolState(fView, layer, fDocument));
	fView->SetCommandStack(fDocument->CommandStack());

	fDocument->CommandStack()->AddListener(&fCommandStackListener);
	_ObjectChanged(fDocument->CommandStack());
}

// destructor
Window::~Window()
{
	fDocument->CommandStack()->RemoveListener(&fCommandStackListener);
	delete fRenderManager;
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
