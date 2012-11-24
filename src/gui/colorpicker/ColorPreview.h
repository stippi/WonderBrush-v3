/* 
 * Copyright 2001 Werner Freytag - please read to the LICENSE file
 *
 * Copyright 2002-2012, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 *		
 */

#ifndef _COLOR_PREVIEW_H
#define _COLOR_PREVIEW_H


#include <Control.h>

#include "PlatformViewMixin.h"


#define	MSG_COLOR_PREVIEW	'ColP'
#define MSG_MESSAGERUNNER 	'MsgR'

class BMessageRunner;

class ColorPreview : public PlatformViewMixin<BControl> {
public:

								ColorPreview(BRect frame, rgb_color color);
								ColorPreview(rgb_color color);
	virtual						~ColorPreview();

	// BControl interface
	virtual	BSize				MinSize();
	virtual	BSize				PreferredSize();
	virtual	BSize				MaxSize();

	virtual	void				AttachedToWindow();
	virtual	void				PlatformDraw(PlatformDrawContext& drawContext);

	virtual	void				MessageReceived(BMessage* message);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
										   const BMessage* message);

	virtual	status_t			Invoke(BMessage* message = NULL);

	// ColorPreview
			void				SetColor(rgb_color color);
									// changes the displayed color
			void				SetNewColor(rgb_color color);
									// changes also the old color

private:
			class PlatformDelegate;

private:
			PlatformDelegate*	fPlatformDelegate;

			void				_DragColor(BPoint where);

			rgb_color			fColor;
			rgb_color			fOldColor;

			bool				fMouseDown;

			BMessageRunner*		fMessageRunner;
};

#endif // _COLOR_PREVIEW_H

