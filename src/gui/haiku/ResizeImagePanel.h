/*
 * Copyright 2015, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef USER_LOGIN_WINDOW_H
#define USER_LOGIN_WINDOW_H

#include <Locker.h>
#include <Messenger.h>
#include <Window.h>

#include "Document.h"


class BButton;
class BCheckBox;
class BMenu;
class BMenuField;
class BTextControl;


class ResizeImagePanel : public BWindow {
public:
								ResizeImagePanel(BWindow* parent, BRect frame);
	virtual						~ResizeImagePanel();

	virtual	void				MessageReceived(BMessage* message);

			void				SetMessage(
									const BMessenger& messenger,
									const BMessage& message);

			void				SetDocument(const DocumentRef& document);

private:
			void				_FillCommonSizes(BMenu* menu) const;
			void				_PrepareNumberTextControl(BTextControl* control);
			void				_AdoptValuesToControls();
			void				_AdoptControlsToValues(bool widthChanged);
			void				_LockAspectRatio(bool adjustWidth);
			void				_Quit(bool success);


private:
			BMessenger			fTarget;
			BMessage			fMessage;

			DocumentRef			fDocument;
			int32				fWidth;
			int32				fHeight;

			BTextControl*		fWidthField;
			BTextControl*		fHeightField;
			BCheckBox*			fLockAspectField;
			BMenuField*			fCommonSizesField;

			BButton*			fCancelButton;
			BButton*			fOKButton;
};


#endif // USER_LOGIN_WINDOW_H
