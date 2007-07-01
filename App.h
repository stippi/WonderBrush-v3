#ifndef APP_H
#define APP_H

#include <Application.h>
#include <Rect.h>

class Document;

enum {
	MSG_NEW_WINDOW	= 'nwnd'
};

class App : public BApplication {
 public:
								App(BRect bounds);

	virtual	void				MessageReceived(BMessage* message);
	virtual void				ReadyToRun();

 private:
			void				_NewWindow();

			Document*			fDocument;

			BRect				fWindowFrame;
			int32				fWindowCount;
};

#endif // APP_H
