#include "BScreen.h"

#include <Window.h>

#include <QApplication>
#include <QDesktopWidget>


BScreen::BScreen(screen_id id)
	:
	fScreenIndex(id.id)
{
}


BScreen::BScreen(BWindow* window)
	:
	fScreenIndex(QApplication::desktop()->screenNumber(window))
{
}


BScreen::~BScreen()
{
}


bool
BScreen::IsValid()
{
	QDesktopWidget* desktop = QApplication::desktop();
	return fScreenIndex == -1
		|| (fScreenIndex >= 0 && fScreenIndex < desktop->screenCount());
}


BRect
BScreen::Frame()
{
	return BRect::FromQRect(
		QApplication::desktop()->screenGeometry(fScreenIndex));
}
