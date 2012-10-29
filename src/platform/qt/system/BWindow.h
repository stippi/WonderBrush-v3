#ifndef BWINDOW_H
#define BWINDOW_H


#include <Looper.h>

#include <QMainWindow>


class BMessage;
class BMessageFilter;


class BWindow : public QMainWindow, public BLooper {
	Q_OBJECT

public:
								BWindow(QWidget* parent = NULL);
	virtual						~BWindow();

	virtual	void				DispatchMessage(BMessage* message,
									BHandler* handler);

private:
			struct ViewAncestryTracker;

private:
			void				_WidgetAdded(QWidget* widget);
			void				_WidgetRemoved(QWidget* widget);
};


#endif // BWINDOW_H
