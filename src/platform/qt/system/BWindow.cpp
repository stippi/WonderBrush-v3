#include "BWindow.h"


BWindow::BWindow(QWidget* parent)
	:
	QMainWindow(parent)
{
}


BMessage*
BWindow::CurrentMessage() const
{
// TODO:...
	return NULL;
}


void
BWindow::AddCommonFilter(BMessageFilter* filter)
{
// TODO:...
}


bool
BWindow::RemoveCommonFilter(BMessageFilter* filter)
{
// TODO:...
return false;
}
