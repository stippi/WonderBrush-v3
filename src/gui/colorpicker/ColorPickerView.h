/*
 * Copyright 2001 Werner Freytag - please read to the LICENSE file
 *
 * Copyright 2002-2006, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 *
 */

#ifndef COLOR_PICKER_VIEW_H
#define COLOR_PICKER_VIEW_H

#include <View.h>

#if LIB_LAYOUT
#  include <layout.h>
#endif

#include "PlatformViewMixin.h"
#include "SelectedColorMode.h"

#define	MSG_RADIOBUTTON					'Rad0'
#define	MSG_TEXTCONTROL					'Txt0'
#define MSG_HEXTEXTCONTROL				'HTxt'
#define MSG_UPDATE_COLOR_PICKER_VIEW	'UpCp'

class ColorField;
class ColorSlider;
class ColorPreview;

class BRadioButton;
class BTextControl;

class ColorPickerView :
						#if LIB_LAYOUT
						public MView,
						#endif
						public PlatformViewMixin<BView> {
 public:
								ColorPickerView(const char* name,
												rgb_color color,
												SelectedColorMode mode);
	virtual						~ColorPickerView();

	#if LIB_LAYOUT
								// MView
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect frame);
	#endif

								// BView
	virtual	void				AttachedToWindow();
	virtual	void				MessageReceived(BMessage *message);

	virtual	void				PlatformDraw(PlatformDrawContext& drawContext);

								// ColorPickerView
			void				SetColorMode(SelectedColorMode mode,
											 bool update = true);
			void				SetColor(rgb_color color);
			rgb_color			Color();
			SelectedColorMode	Mode() const
									{ return fSelectedColorMode; }

private:
			class PlatformDelegate;
			friend class PlatformDelegate;

private:
			int32				_NumForMode(SelectedColorMode mode) const;

			void				_UpdateColor(float value, float value1,
											 float value2);
			void				_UpdateTextControls();

private:
			PlatformDelegate*	fPlatformDelegate;

	SelectedColorMode			fSelectedColorMode;

	float						h, s, v, r, g, b;
	float						*p, *p1, *p2;

	bool						fRequiresUpdate;

	ColorField*					fColorField;
	ColorSlider*				fColorSlider;
	ColorPreview*				fColorPreview;
};

#endif // COLOR_PICKER_VIEW_H


