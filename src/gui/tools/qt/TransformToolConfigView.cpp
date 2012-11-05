#include "TransformToolConfigView.h"
#include "ui_TransformToolConfigView.h"

#include "TransformTool.h"
#include "TransformToolState.h"


TransformToolConfigView::TransformToolConfigView(::Tool* tool)
	:
	ToolConfigView(tool),
	fUi(new Ui::TransformToolConfigView),
	fNotificationsEnabled(true)
{
	fUi->setupUi(this);

	connect(fUi->translationXEdit, SIGNAL(valueChanged(double)),
		SLOT(_TranslationXChanged()));
	connect(fUi->translationYEdit, SIGNAL(valueChanged(double)),
		SLOT(_TranslationYChanged()));
	connect(fUi->rotationEdit, SIGNAL(valueChanged(double)),
		SLOT(_RotationChanged()));
	connect(fUi->scaleXEdit, SIGNAL(valueChanged(double)),
		SLOT(_ScaleXChanged()));
	connect(fUi->scaleYEdit, SIGNAL(valueChanged(double)),
		SLOT(_ScaleYChanged()));
	connect(fUi->subpixelsCheckBox, SIGNAL(stateChanged(int)),
		SLOT(_SubpixelsChanged()));
}


TransformToolConfigView::~TransformToolConfigView()
{
	delete fUi;
}


// #pragma mark -


void
TransformToolConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_TRANSFORMATION_CHANGED:
		{
			BPoint pivot;
			BPoint translation;
			double rotation;
			double scaleX;
			double scaleY;

			message->FindFloat("pivot x", &pivot.x);
			message->FindFloat("pivot y", &pivot.y);
			message->FindFloat("translation x", &translation.x);
			message->FindFloat("translation y", &translation.y);
			message->FindDouble("rotation", &rotation);
			message->FindDouble("scale x", &scaleX);
			message->FindDouble("scale y", &scaleY);

			fNotificationsEnabled = false;

			_SetValue(fUi->translationXEdit, translation.x);
			_SetValue(fUi->translationYEdit, translation.y);
			_SetValue(fUi->rotationEdit, rotation);
			_SetValue(fUi->scaleXEdit, scaleX);
			_SetValue(fUi->scaleYEdit, scaleY);

			fNotificationsEnabled = true;
			break;
		}

		default:
			ToolConfigView::MessageReceived(message);
			break;
	}
}


void
TransformToolConfigView::_TranslationXChanged()
{
	if (fNotificationsEnabled) {
		fTool->SetOption(TransformTool::TRANSLATION_X,
			(float)fUi->translationXEdit->value());
	}
}


void
TransformToolConfigView::_TranslationYChanged()
{
	if (fNotificationsEnabled) {
		fTool->SetOption(TransformTool::TRANSLATION_Y,
			(float)fUi->translationYEdit->value());
	}
}


void
TransformToolConfigView::_RotationChanged()
{
	if (fNotificationsEnabled) {
		fTool->SetOption(TransformTool::ROTATION,
			(float)fUi->rotationEdit->value());
	}
}


void
TransformToolConfigView::_ScaleXChanged()
{
	if (fNotificationsEnabled) {
		fTool->SetOption(TransformTool::SCALE_X,
			(float)fUi->scaleXEdit->value());
	}
}


void
TransformToolConfigView::_ScaleYChanged()
{
	if (fNotificationsEnabled) {
		fTool->SetOption(TransformTool::SCALE_Y,
			(float)fUi->scaleYEdit->value());
	}
}


void
TransformToolConfigView::_SubpixelsChanged()
{
	if (fNotificationsEnabled) {
		fTool->SetOption(TransformTool::SUBPIXELS,
			(float)fUi->subpixelsCheckBox->isChecked());
	}
}


// #pragma mark -


void
TransformToolConfigView::UpdateStrings()
{
}


void
TransformToolConfigView::SetActive(bool active)
{
}


void
TransformToolConfigView::SetEnabled(bool enable)
{
	fUi->translationXEdit->setEnabled(enable);
	fUi->translationYEdit->setEnabled(enable);
	fUi->rotationEdit->setEnabled(enable);
	fUi->scaleXEdit->setEnabled(enable);
	fUi->scaleYEdit->setEnabled(enable);
	fUi->subpixelsCheckBox->setEnabled(enable);
}


// #pragma mark - private


void
TransformToolConfigView::_SetValue(QDoubleSpinBox* control, float value) const
{
#if 0
	char text[64];
	snprintf(text, sizeof(text), "%.2f", value);
	int32 selectionStart;
	int32 selectionEnd;
	control->TextView()->GetSelection(&selectionStart, &selectionEnd);
	bool selectionEndIsTextEnd
		= selectionEnd == control->TextView()->TextLength();

	control->SetText(text);

	if (selectionEndIsTextEnd)
		selectionEnd = control->TextView()->TextLength();
	control->TextView()->Select(selectionStart, selectionEnd);
#endif
	control->setValue(value);
	// TODO: Keep selection!

}
