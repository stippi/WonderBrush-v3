/*
 * Copyright 2001-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef ICON_OPTIONS_CONTROL_H
#define ICON_OPTIONS_CONTROL_H

#include <Control.h>
#include <Invoker.h>

class BGroupView;
class IconButton;

class IconOptionsControl : public BControl {
public:
								IconOptionsControl(const char* name = NULL,
									const char* label = NULL,
									BMessage* message = NULL,
									BHandler* target = NULL,
									enum orientation orientation
										= B_HORIZONTAL);
								~IconOptionsControl();

	// BControl interface
	virtual	void				AttachedToWindow();
	virtual	void				AllAttached();
	virtual	void				Draw(BRect updateRect);
	virtual	void				FrameResized(float width, float height);
	virtual	void				GetPreferredSize(float* width, float* height);
	virtual	BSize				MinSize();
	virtual	BSize				MaxSize();
	virtual	BSize				PreferredSize();
	virtual	void				InvalidateLayout(bool descendants);

	virtual	void				SetValue(int32 value);
	virtual	void				SetEnabled(bool enable);

	virtual	void				MessageReceived(BMessage* message);

	// BInvoker interface
	virtual	status_t			Invoke(BMessage* message = NULL);

	// IconOptionsControl
			void				AddOption(IconButton* icon);

			BLayoutItem*		CreateLabelLayoutItem();
			BLayoutItem*		CreateIconBarLayoutItem();

protected:
	virtual	void				DoLayout();

private:
			class LabelLayoutItem;
			class IconBarLayoutItem;
 			struct LayoutData;

			friend class LabelLayoutItem;
			friend class IconBarLayoutItem;
			friend class LayoutData;

			IconButton*			_FindIcon(int32 index) const;

			void				_ValidateLayoutData();
			void				_UpdateFrame();

			BHandler*			fTargetCache;
			enum orientation	fOrientation;

			LayoutData*			fLayoutData;
			BGroupView*			fIconGroup;
};

#endif // ICON_OPTIONS_CONTROL_H
