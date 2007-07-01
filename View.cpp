#include "View.h"

#include <Message.h>
#include <Messenger.h>

#include "Document.h"
#include "RenderManager.h"

// #pragma mark -

// constructor
View::View(BRect frame, Document* document, RenderManager* manager)
	: BackBufferedStateView(frame, "view", B_FOLLOW_ALL, B_WILL_DRAW)
	, fDocument(document)
	, fRenderManager(manager)
{
}

// destructor
View::~View()
{
}

// AttachedToWindow
void
View::AttachedToWindow()
{
	BackBufferedStateView::AttachedToWindow();
	fRenderManager->SetBitmapListener(new BMessenger(this));
}

// MessageReceived
void
View::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_BITMAP_CLEAN: {
			BRect area;
			if (message->FindRect("area", &area) == B_OK)
				Invalidate(area);
			break;
		}

		default:
			BackBufferedStateView::MessageReceived(message);
	}
}

// #pragma mark -

// DrawInto
void
View::DrawInto(BView* view, BRect updateRect)
{
	// draw document bitmap
	if (!fRenderManager->LockDisplay())
		return;

	view->DrawBitmap(fRenderManager->DisplayBitmap(),
		updateRect, updateRect);

	fRenderManager->UnlockDisplay();

	// draw ViewState
	BackBufferedStateView::DrawInto(view, updateRect);
}

