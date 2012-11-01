/*
 * Copyright 2006-2012 Stephan Aßmus <superstippi@gmx.de>
 * Distributed under the terms of the MIT License.
 */

#ifndef SWATCH_GROUP_H
#define SWATCH_GROUP_H

#include <View.h>

#include "Listener.h"
#include "SelectedColorMode.h"

class AlphaSlider;
class ColorField;
class ColorPickerPanel;
class ColorSlider;
class SwatchView;

class SwatchGroup : public BView, public Listener {
public:
								SwatchGroup(const char* name);
	virtual						~SwatchGroup();

	// Listener interface
	virtual	void				ObjectChanged(const Notifier* object);

	// BView interface
	virtual	void				AttachedToWindow();
	virtual	void				MessageReceived(BMessage* message);

	// SwatchGroup
//			void				SetCurrentColor(CurrentColor* color);

private:
			void				_SetColor(rgb_color color);
			void				_AdoptColor(rgb_color color);
			void				_AdoptColor(float h, float s, float v,
									uint8 a);

private:
			SwatchView*			fCurrentColorSV;
			SwatchView*			fSwatchViews[20];
			ColorField*			fColorField;
			ColorSlider*		fColorSlider;
			AlphaSlider*		fAlphaSlider;

//			CurrentColor*		fCurrentColor;
			bool				fIgnoreNotifications;

			ColorPickerPanel*	fColorPickerPanel;
			SelectedColorMode fColorPickerMode;
			BRect				fColorPickerFrame;
};

#endif // SWATCH_GROUP_H
