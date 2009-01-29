#include "TextViewPopup.h"

#include <stdio.h>

#include <Message.h>
#include <TextView.h>
#include <Window.h>

class TextViewParent : public BView {
 public:
								TextViewParent(BRect frame);
	virtual						~TextViewParent();
	virtual	void				FrameResized(float width, float height);
};

// constructor
TextViewParent::TextViewParent(BRect frame)
	: BView(frame, "text view parent", B_FOLLOW_NONE, B_FRAME_EVENTS)
{
}

// destructor
TextViewParent::~TextViewParent()
{
}

// FrameResized
void
TextViewParent::FrameResized(float width, float height)
{
	Window()->ResizeTo(width, height);
}

// #pragma mark -

class PopupTextView : public BTextView {
 public:
								PopupTextView(BRect frame,
											  const BString& text,
											  TextViewPopup* popup);
	virtual						~PopupTextView();

	virtual	void				AttachedToWindow();
	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				MouseDown(BPoint where);
	virtual	void				MakeFocus(bool focus = true);

	// InputTextView
	virtual	void				RevertChanges();
	virtual	void				ApplyChanges(int32 next = 0);

 private:
			TextViewPopup*		fPopup;
};

// constructor
PopupTextView::PopupTextView(BRect frame, const BString& text,
							 TextViewPopup* popup)
	: BTextView(frame, "popup text view", frame.InsetByCopy(2, 1),
				B_FOLLOW_ALL, B_WILL_DRAW)
	, fPopup(popup)
{
	SetText(text.String());
}

// destructor
PopupTextView::~PopupTextView()
{
	delete fPopup;
}

// AttachedToWindow
void
PopupTextView::AttachedToWindow()
{
	MakeResizable(true, Parent());
	SelectAll();
	MakeFocus();
	SetEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
}

// KeyDown
void
PopupTextView::KeyDown(const char* bytes, int32 numBytes)
{
	bool handled = true;
	if (numBytes > 0) {
		switch (bytes[0]) {
			case B_ESCAPE:
				// revert any typing changes
				RevertChanges();
				break;
			case B_TAB: {
				// skip BTextView implementation
				BView::KeyDown(bytes, numBytes);
				int32 next = modifiers() & B_SHIFT_KEY ? -1 : 1;
				ApplyChanges(next);
				break;
			}
			case B_RETURN:
				ApplyChanges();
				break;
			default:
				handled = false;
				break;
		}
	}
	if (!handled)
		BTextView::KeyDown(bytes, numBytes);
}

// MouseDown
void
PopupTextView::MouseDown(BPoint where)
{
	BRect bounds(Bounds());
	bounds.InsetBy(-1, -1);
	if (!bounds.Contains(where))
		ApplyChanges();
	else
		BTextView::MouseDown(where);
}

// MakeFocus
void
PopupTextView::MakeFocus(bool focus)
{
	BTextView::MakeFocus(focus);
	if (!focus)
		ApplyChanges();
}

// #pragma mark -

// RevertChanges
void
PopupTextView::RevertChanges()
{
	fPopup->Cancel();
}

// ApplyChanges
void
PopupTextView::ApplyChanges(int32 next)
{
	fPopup->TextEdited(Text(), next);
}

// #pragma mark -

// constructor
TextViewPopup::TextViewPopup(BRect frame, const BString& text,
							 BMessage* message, BHandler* target)
	: fMessage(message)
	, fTarget(target)
	, fCanceled(false)
{
	fPopupWindow = new BWindow(frame, "text edit popup",
							   B_BORDERED_WINDOW_LOOK,
							   B_MODAL_APP_WINDOW_FEEL,
							   B_ASYNCHRONOUS_CONTROLS);

	frame.OffsetTo(B_ORIGIN);
	fTextView = new PopupTextView(frame, text, this);
	BView* textViewParent = new TextViewParent(frame);
	textViewParent->AddChild(fTextView);
	fPopupWindow->AddChild(textViewParent);
	fPopupWindow->Show();
}

// destructor
TextViewPopup::~TextViewPopup()
{
	delete fMessage;
}

// TextEdited
void
TextViewPopup::TextEdited(const char* text, int32 next)
{
	if (fCanceled)
		return;

	if (fMessage && fTarget.IsValid()) {
		fMessage->AddString("text", text);
		if (next != 0)
			fMessage->AddInt32("next", next);
		fTarget.SendMessage(fMessage);
	}
	fPopupWindow->PostMessage(B_QUIT_REQUESTED);
}

// Cancel
void
TextViewPopup::Cancel()
{
	fCanceled = true;
	if (fMessage && fTarget.IsValid()) {
		fTarget.SendMessage(fMessage);
	}
	fPopupWindow->PostMessage(B_QUIT_REQUESTED);
}
