/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef PATH_TOOL_CONFIG_VIEW_H
#define PATH_TOOL_CONFIG_VIEW_H

#include "ToolConfigView.h"

class BCheckBox;

class PathToolConfigView : public ToolConfigView {
public:
								PathToolConfigView(::Tool* tool);
	virtual						~PathToolConfigView();

	// ToolConfigView interface
	virtual	void				UpdateStrings();
	virtual	void				SetActive(bool active);
	virtual	void				SetEnabled(bool enable);

	// BView interface
	virtual	void				AttachedToWindow();
	virtual	void				DetachedFromWindow();
	virtual	void				MessageReceived(BMessage* message);

private:
			BCheckBox*			fSubpixels;
};

#endif // PATH_TOOL_CONFIG_VIEW_H
