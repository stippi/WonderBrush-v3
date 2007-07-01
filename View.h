#ifndef VIEW_H
#define VIEW_H

#include "AbstractLOAdapter.h"
#include "BackBufferedStateView.h"

class Document;
class RenderManager;

class View : public BackBufferedStateView {
 public:
								View(BRect frame,
									Document* document,
									RenderManager* manager);
	virtual						~View();

	// BView interface
	virtual	void				AttachedToWindow();
	virtual	void				MessageReceived(BMessage* message);

	// BackBufferedStateView interface
	virtual void				DrawInto(BView* view, BRect updateRect);

 private:
			Document*			fDocument;
			RenderManager*		fRenderManager;
};

#endif // VIEW_H
