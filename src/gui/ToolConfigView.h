/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef CONFIG_VIEW_H
#define CONFIG_VIEW_H

#include <View.h>

class Tool;

class ToolConfigView : public BView {
public:
								ToolConfigView(::Tool* tool);
	virtual						~ToolConfigView();

	// ie the interface language has changed
	virtual	void				UpdateStrings();
	// user switched tool
	virtual	void				SetActive(bool active);
	// en/disable controls
	virtual	void				SetEnabled(bool enable);

			::Tool*				Tool() const
									{ return fTool; }

protected:
			::Tool*				fTool;
};

#endif // CONFIG_VIEW_H
