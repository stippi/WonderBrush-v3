#ifndef TEXT_VIEW_POPUP_H
#define TEXT_VIEW_POPUP_H

#include <Messenger.h>
#include <Rect.h>
#include <String.h>

class BHandler;
class BMessage;
class BWindow;
class PopupTextView;

class TextViewPopup {
 public:
								TextViewPopup(BRect frame,
											  const BString& text,
											  BMessage* message,
											  BHandler* target);

			void				Cancel();

 private:
	friend class PopupTextView;

	virtual						~TextViewPopup();

			void				TextEdited(const char* text,
										   int32 next = 0);

			PopupTextView*		fTextView;
			BWindow*			fPopupWindow;
			BMessage*			fMessage;
			BMessenger			fTarget;
			bool				fMessageSent;
};

#endif // TEXT_VIEW_POPUP_H
