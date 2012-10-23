#ifndef BWINDOW_H
#define BWINDOW_H


#include <QMainWindow>


class BMessage;
class BMessageFilter;


class BWindow : public QMainWindow
{
public:
								BWindow(QWidget* parent = NULL);

	// TODO: The following is actually BLooper functionality!
			BMessage*			CurrentMessage() const;

	virtual	void			AddCommonFilter(BMessageFilter* filter);
	virtual	bool			RemoveCommonFilter(BMessageFilter* filter);
};


#endif // BWINDOW_H
