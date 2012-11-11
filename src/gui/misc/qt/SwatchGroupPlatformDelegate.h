#ifndef SWATCH_GROUP_PLATFORM_DELEGATE_H
#define SWATCH_GROUP_PLATFORM_DELEGATE_H


#include "SwatchGroup.h"

#include <QHBoxLayout>
#include <QFrame>
#include <QVBoxLayout>

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
// TODO: Weights!
		QVBoxLayout* layout = new QVBoxLayout(fView);
		layout->setSpacing(spacing);
		layout->setMargin(spacing);

		QHBoxLayout* swatchLayout = new QHBoxLayout();
		swatchLayout->setSpacing(spacing);
		layout->addLayout(swatchLayout);
		swatchLayout->addWidget(fView->fCurrentColorSV);

		QVBoxLayout* swatchViewsLayout = new QVBoxLayout();
		swatchViewsLayout->setSpacing(spacing);
		swatchLayout->addLayout(swatchViewsLayout);

		for (int i = 0; i < 2; i++) {
			QHBoxLayout* swatchViewsRowLayout = new QHBoxLayout();
			swatchViewsRowLayout->setSpacing(spacing);
			swatchViewsLayout->addLayout(swatchViewsRowLayout);
			for (int k = 0; k < 10; k++) {
				swatchViewsRowLayout->addWidget(
					fView->fSwatchViews[i * 10 + k]);
			}
		}

		QFrame* line = new QFrame();
		line->setFrameShape(QFrame::HLine);
		layout->addWidget(line);

		layout->addWidget(fView->fColorField);

		line = new QFrame();
		line->setFrameShape(QFrame::HLine);
		layout->addWidget(line);

		layout->addWidget(fView->fColorSlider);

		line = new QFrame();
		line->setFrameShape(QFrame::HLine);
		layout->addWidget(line);

		layout->addWidget(fView->fAlphaSlider);
	}

private:
	SwatchGroup*	fView;
};


#endif // SWATCH_GROUP_PLATFORM_DELEGATE_H
