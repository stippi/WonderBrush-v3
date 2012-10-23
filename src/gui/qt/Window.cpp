#include "Window.h"
#include "ui_Window.h"

#include "CanvasView.h"
#include "CommandStack.h"
#include "Document.h"
#include "RenderManager.h"


Window::Window(BRect frame, const char* title, Document* document, Layer* layer,
	QWidget* parent)
	:
	QMainWindow(parent),
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

//	_InitTools();

//	fView->SetCommandStack(fDocument->CommandStack());

	fDocument->CommandStack()->AddListener(&fCommandStackListener);
//	_ObjectChanged(fDocument->CommandStack());
}


Window::~Window()
{
	delete fUi;
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
