#include "BrushToolConfigView.h"
#include "ui_BrushToolConfigView.h"

#include <Message.h>

#include "BrushTool.h"


enum {
	MSG_OPACITY_VALUE		= 'opvl',
	MSG_OPACITY_CONTROL		= 'opcn',

	MSG_RADIUS_VALUE		= 'rdvl',
	MSG_RADIUS_CONTROL		= 'rdcn',

	MSG_HARDNESS_VALUE		= 'hdvl',
	MSG_HARDNESS_CONTROL	= 'hdcn',

	MSG_SPACING_VALUE		= 'spvl',

	MSG_SUBPIXELS			= 'sbpx',
	MSG_SOLID				= 'slid',
	MSG_TILT				= 'tilt',
};


BrushToolConfigView::BrushToolConfigView(::Tool* tool) :
	ToolConfigView(tool),
	fUi(new Ui::BrushToolConfigView)
{
	fUi->setupUi(this);

	fUi->opacitySlider->Init("Opacity",
		new BMessage(MSG_OPACITY_VALUE),
		new BMessage(MSG_OPACITY_CONTROL), this);

	fUi->radiusSlider->Init("Radius",
		new BMessage(MSG_RADIUS_VALUE),
		new BMessage(MSG_RADIUS_CONTROL), this, 0.0f, 1.0f / 100.0f);

	fUi->hardnessSlider->Init("Hardness",
		new BMessage(MSG_HARDNESS_VALUE),
		new BMessage(MSG_HARDNESS_CONTROL), this);
	fUi->hardnessSlider->SetMinEnabled(false);

	fUi->spacingSlider->Init("Spacing",
		new BMessage(MSG_SPACING_VALUE), NULL, this, 0.0f, 0.1f);
	fUi->spacingSlider->SetMinEnabled(false);
fUi->spacingSlider->SetEnabled(false);

	connect(fUi->subpixelsCheckBox, SIGNAL(stateChanged(int)),
		SLOT(_SubpixelsChanged()));
	connect(fUi->solidCheckBox, SIGNAL(stateChanged(int)),
		SLOT(_SolidChanged()));
	connect(fUi->tiltCheckBox, SIGNAL(stateChanged(int)),
		SLOT(_TiltChanged()));
}


BrushToolConfigView::~BrushToolConfigView()
{
	delete fUi;
}


// #pragma mark -

// AttachedToWindow
void
BrushToolConfigView::AttachedToWindow()
{
#if 0
	fSubpixels->SetTarget(this);
	fSolid->SetTarget(this);
	fTilt->SetTarget(this);
#endif
}


// MessageReceived
void
BrushToolConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_OPACITY_VALUE:
			_SetMinMaxOption(message, BrushTool::OPACITY_MIN,
				BrushTool::OPACITY_MAX);
			break;

		case MSG_OPACITY_CONTROL:
			Tool()->SetOption(BrushTool::OPACITY_CONTROLLED,
				fUi->opacitySlider->IsMinEnabled());
			break;

		case MSG_RADIUS_VALUE:
			_SetMinMaxOption(message, BrushTool::RADIUS_MIN,
				BrushTool::RADIUS_MAX);
			break;

		case MSG_RADIUS_CONTROL:
			Tool()->SetOption(BrushTool::RADIUS_CONTROLLED,
				fUi->radiusSlider->IsMinEnabled());
			break;

		case MSG_HARDNESS_VALUE:
			_SetMinMaxOption(message, BrushTool::HARDNESS_MIN,
				BrushTool::HARDNESS_MAX);
			break;

		case MSG_HARDNESS_CONTROL:
			Tool()->SetOption(BrushTool::HARDNESS_CONTROLLED,
				fUi->hardnessSlider->IsMinEnabled());
			break;

		case MSG_SPACING_VALUE:
			_SetMinMaxOption(message, BrushTool::SPACING,
				BrushTool::SPACING);
			break;

		default:
			ToolConfigView::MessageReceived(message);
			break;
	}
}

// #pragma mark -

// UpdateStrings
void
BrushToolConfigView::UpdateStrings()
{
}

// SetActive
void
BrushToolConfigView::SetActive(bool active)
{
}

// SetEnabled
void
BrushToolConfigView::SetEnabled(bool enable)
{
	fUi->opacitySlider->SetEnabled(enable);
	fUi->radiusSlider->SetEnabled(enable);
	fUi->hardnessSlider->SetEnabled(enable);
//	fUi->spacingSlider->SetEnabled(enable);
	fUi->subpixelsCheckBox->setEnabled(enable);
//	fUi->solidCheckBox->setEnabled(enable);
	fUi->tiltCheckBox->setEnabled(enable);
}

// #pragma mark -

// _SetMinMaxOption
void
BrushToolConfigView::_SetMinMaxOption(const BMessage* message,
	uint32 minOption, uint32 maxOption)
{
	float min;
	float max;
	if (message->FindFloat("min value", &min) == B_OK
		&& message->FindFloat("max value", &max) == B_OK) {
		if (minOption != maxOption)
			Tool()->SetOption(minOption, min);
		Tool()->SetOption(maxOption, max);
	}
}


void
BrushToolConfigView::_SubpixelsChanged()
{
	Tool()->SetOption(BrushTool::SUBPIXELS,
		fUi->subpixelsCheckBox->isChecked());
}


void
BrushToolConfigView::_SolidChanged()
{
	Tool()->SetOption(BrushTool::SOLID, fUi->solidCheckBox->isChecked());
}


void
BrushToolConfigView::_TiltChanged()
{
	Tool()->SetOption(BrushTool::TILT_CONTROLLED,
		fUi->tiltCheckBox->isChecked());
}
