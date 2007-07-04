#ifndef WINDOW_H
#define WINDOW_H

#include <Window.h>

#include "ListenerAdapter.h"

class Document;
class Layer;
class RenderManager;
class View;

class Window : public BWindow {
 public:
								Window(BRect frame, const char* title,
									Document* document, Layer* layer);
	virtual						~Window();

	// BWindow interface
	virtual	bool				QuitRequested();
	virtual	void				MessageReceived(BMessage* message);

	// Window
			void				SetDocument(Document* document);

 private:
			void				_ObjectChanged(const Notifier* object);


			View*				fView;
			Document*			fDocument;
			RenderManager*		fRenderManager;
			ListenerAdapter		fCommandStackListener;

			BMenuItem*			fUndoMI;
			BMenuItem*			fRedoMI;
};

#endif // WINDOW_H
