#include "BWindow.h"

#include <typeinfo>

#include <View.h>

#include <QCoreApplication>
#include <QEvent>


struct BWindow::ViewAncestryTracker : public QObject {
	ViewAncestryTracker() :
		QObject(QCoreApplication::instance())
	{
	}

	static ViewAncestryTracker* GetTracker()
	{
		static ViewAncestryTracker* tracker = new ViewAncestryTracker;
		return tracker;
	}

	void RegisterWindow(BWindow* window)
	{
		QMutexLocker mutexLocker(&fMutex);
		bool installFilter = fWindows.isEmpty();
		fWindows.insert(window);
		if (installFilter)
			QCoreApplication::instance()->installEventFilter(this);
	}

	void UnregisterWindow(BWindow* window)
	{
		QMutexLocker mutexLocker(&fMutex);
		fWindows.remove(window);
		if (fWindows.isEmpty())
			QCoreApplication::instance()->removeEventFilter(this);
	}

protected:
	virtual bool eventFilter(QObject* watched, QEvent* event)
	{
		if (event->type() == QEvent::ParentChange) {
			if (QWidget* widget = dynamic_cast<QWidget*>(watched)) {
				QMutexLocker mutexLocker(&fMutex);
				BWindow* oldWindow = fWidgets.value(widget, NULL);
				BWindow* newWindow = dynamic_cast<BWindow*>(widget->window());
				if (oldWindow != newWindow) {
					if (newWindow == NULL)
						fWidgets.remove(widget);
					else
						fWidgets.insert(widget, newWindow);
					mutexLocker.unlock();

					if (oldWindow != NULL)
						oldWindow->_WidgetRemoved(widget);
					if (newWindow != NULL)
						newWindow->_WidgetAdded(widget);
				}
			}
		}

		return false;
	}

private:
	QMutex						fMutex;
	QSet<BWindow*>				fWindows;
	QHash<QWidget*, BWindow*>	fWidgets;
};


BWindow::BWindow(QWidget* parent)
	:
	QMainWindow(parent),
	BLooper()
{
	ObjectConstructed(this);

	ViewAncestryTracker::GetTracker()->RegisterWindow(this);
}


BWindow::~BWindow()
{
	ViewAncestryTracker::GetTracker()->RegisterWindow(this);

	ObjectAboutToBeDestroyed(this);
}


void
BWindow::_WidgetAdded(QWidget* widget)
{
	BView* view = dynamic_cast<BView*>(widget);
	if (view != NULL)
		view->_AttachToWindow(this);

	foreach (QObject* child, widget->children()) {
		if (QWidget* childWidget = dynamic_cast<QWidget*>(child))
			_WidgetAdded(childWidget);
	}

	if (view != NULL)
		view->_AllAttachedToWindow();
}


void
BWindow::_WidgetRemoved(QWidget* widget)
{
	BView* view = dynamic_cast<BView*>(widget);
	if (view != NULL)
		view->_DetachFromWindow();

	foreach (QObject* child, widget->children()) {
		if (QWidget* childWidget = dynamic_cast<QWidget*>(child))
			_WidgetRemoved(childWidget);
	}

	if (view != NULL)
		view->_AllDetachedFromWindow();
}
