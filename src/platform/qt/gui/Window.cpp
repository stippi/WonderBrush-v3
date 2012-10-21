#include "Window.h"
#include "ui_Window.h"

#include "CommandStack.h"
#include "Document.h"


Window::Window(BRect frame, const char* title, Document* document, Layer* layer,
	QWidget* parent)
	:
	QMainWindow(parent),
	fUi(new Ui::Window),
	fDocument(document),
	fRenderManager(NULL),
// TODO:...
//	fCommandStackListener(this)
	fCommandStackListener(NULL)
{
	fUi->setupUi(this);

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
