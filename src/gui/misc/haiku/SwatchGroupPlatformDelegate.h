/*
 * Copyright 2006-2012 Stephan Aßmus <superstippi@gmx.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef SWATCH_GROUP_PLATFORM_DELEGATE_H
#define SWATCH_GROUP_PLATFORM_DELEGATE_H


#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <SeparatorView.h>

#include "SwatchGroup.h"

#include "AlphaSlider.h"
#include "ColorField.h"
#include "ColorSlider.h"
#include "SwatchView.h"


class SwatchGroup::PlatformDelegate {
public:
	PlatformDelegate(SwatchGroup* view)
		:
		fView(view)
	{
	}

	void Init(int spacing)
	{
		fView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

		// layout UI
		BGroupLayout* layout = new BGroupLayout(B_VERTICAL, spacing);
		fView->SetLayout(layout);
		BGroupLayoutBuilder(layout)
			.AddGroup(B_HORIZONTAL, 0.0f)
				.Add(fView->fCurrentColorSV)
				.AddGroup(B_VERTICAL, 0.0f)
					.AddGroup(B_HORIZONTAL, 0.0f)
						.Add(fView->fSwatchViews[0])
						.Add(fView->fSwatchViews[1])
						.Add(fView->fSwatchViews[2])
						.Add(fView->fSwatchViews[3])
						.Add(fView->fSwatchViews[4])
						.Add(fView->fSwatchViews[5])
						.Add(fView->fSwatchViews[6])
						.Add(fView->fSwatchViews[7])
						.Add(fView->fSwatchViews[8])
						.Add(fView->fSwatchViews[9])
					.End()
					.AddGroup(B_HORIZONTAL, 0.0f)
						.Add(fView->fSwatchViews[10])
						.Add(fView->fSwatchViews[11])
						.Add(fView->fSwatchViews[12])
						.Add(fView->fSwatchViews[13])
						.Add(fView->fSwatchViews[14])
						.Add(fView->fSwatchViews[15])
						.Add(fView->fSwatchViews[16])
						.Add(fView->fSwatchViews[17])
						.Add(fView->fSwatchViews[18])
						.Add(fView->fSwatchViews[19])
					.End()
				.End()
			.End()
			.Add(new BSeparatorView(B_HORIZONTAL))
			.Add(fView->fColorField, 3.0f)
			.Add(new BSeparatorView(B_HORIZONTAL))
			.Add(fView->fColorSlider, 1.0f)
			.Add(new BSeparatorView(B_HORIZONTAL))
			.Add(fView->fAlphaSlider, 1.0f)
			.SetInsets(spacing, spacing, spacing, spacing)
		;
	}

private:
	SwatchGroup*	fView;
};


#endif // SWATCH_GROUP_PLATFORM_DELEGATE_H
