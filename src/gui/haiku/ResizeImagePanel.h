/*
 * Copyright 2015, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef USER_LOGIN_WINDOW_H
#define USER_LOGIN_WINDOW_H

#include <Locker.h>
#include <Messenger.h>
#include <Window.h>


class BButton;
class BCheckBox;
class BMenuField;
class BTextControl;


class ResizeImagePanel : public BWindow {
public:
								ResizeImagePanel(BWindow* parent, BRect frame);
	virtual						~ResizeImagePanel();

	virtual	void				MessageReceived(BMessage* message);

			void				SetOnSuccessMessage(
									const BMessenger& messenger,
									const BMessage& message);

private:
			void				_FillCommonSizes();


private:
			BMessenger			fOnSuccessTarget;
			BMessage			fOnSuccessMessage;

			BTextControl*		fWidthField;
			BTextControl*		fHeightField;
			BCheckBox*			fLockAspectField;
			BMenuField*			fCommonSizesField;

			BButton*			fCancelButton;
			BButton*			fOKButton;
};


#endif // USER_LOGIN_WINDOW_H
