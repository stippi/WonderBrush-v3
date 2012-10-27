/*
 * Copyright 2002-2010 Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2012 Ingo Weinhold <ingo_weinhold@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "DualSlider.h"

#include <Message.h>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QSpacerItem>
#include <QVBoxLayout>

#include "support.h"


#define LABEL_SPACING 2

//static const int kMaxSliderValue = 1 << 31;
	// QSlider is unhappy about this value.
static const int kMaxSliderValue = 1 << 16;


DualSlider::DualSlider(QWidget* parent, const char* label,
		BMessage* valueMessage, BMessage* controlMessage, BHandler* target,
		float minValue, float maxValue)
	:
	BView("", 0),
	fLabel(new QLabel(QString::fromUtf8(label), this)),
	fCheckBox(new QCheckBox(this)),
	fMinSlider(new QSlider(Qt::Horizontal, this)),
	fMaxSlider(new QSlider(Qt::Horizontal, this)),
	fDraggingMinSlider(false),
	fDraggingMaxSlider(false),
	fIgnoreMinSliderChange(false),
	fIgnoreMaxSliderChange(false),
	fMinValue(minValue),
	fMaxValue(maxValue),
	fLastFactor(0.0),
	fValueMessage(valueMessage),
	fControlMessage(controlMessage),
	fTarget(target)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
// TODO: The check box is so big that additional spacing isn't needed ATM.
//	layout->setSpacing(LABEL_SPACING);
	layout->setSpacing(0);

	// first row (label, check box)
	QHBoxLayout* rowLayout = new QHBoxLayout();
	layout->addLayout(rowLayout);
	rowLayout->addWidget(fLabel);
	rowLayout->addItem(new QSpacerItem(5, 0, QSizePolicy::MinimumExpanding,
		QSizePolicy::Fixed));
	rowLayout->addWidget(fCheckBox);

	// second row (min slider)
	rowLayout = new QHBoxLayout();
	layout->addLayout(rowLayout);
	layout->addSpacing(5);
	layout->addWidget(fMinSlider);

	// third row (min slider)
	rowLayout = new QHBoxLayout();
	layout->addLayout(rowLayout);
	layout->addSpacing(5);
	layout->addWidget(fMaxSlider);

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	// sliders
	fMinSlider->setRange(0, kMaxSliderValue);
	fMinSlider->setValue(_ToSliderValue(fMinValue));
	fMaxSlider->setRange(0, kMaxSliderValue);
	fMaxSlider->setValue(_ToSliderValue(fMaxValue));

	// check box
	fCheckBox->setChecked(true);
	fCheckBox->setToolTip(
		QString::fromUtf8("Enables control by pen pressure."));

	connect(fMinSlider, SIGNAL(valueChanged(int)), SLOT(_MinValueChanged()));
	connect(fMinSlider, SIGNAL(sliderPressed()), SLOT(_MinSliderPressed()));
	connect(fMinSlider, SIGNAL(sliderReleased()), SLOT(_MinSliderReleased()));
	connect(fMaxSlider, SIGNAL(valueChanged(int)), SLOT(_MaxValueChanged()));
	connect(fMaxSlider, SIGNAL(sliderPressed()), SLOT(_MaxSliderPressed()));
	connect(fMaxSlider, SIGNAL(sliderReleased()), SLOT(_MaxSliderReleased()));
	connect(fCheckBox, SIGNAL(stateChanged(int)),
		SLOT(_CheckBoxStateChanged()));
}


DualSlider::~DualSlider()
{
	delete fValueMessage;
	delete fControlMessage;
}

void
DualSlider::Init(const char* label, BMessage* valueMessage,
	BMessage* controlMessage, BHandler* target, float minValue, float maxValue)
{
	SetLabel(label);
	delete fValueMessage;
	valueMessage = valueMessage;
	delete fControlMessage;
	fControlMessage = controlMessage;
	fTarget = target;

	fMinValue = minValue;
	fMaxValue = maxValue;
	fMinSlider->setValue(_ToSliderValue(fMinValue));
	fMaxSlider->setValue(_ToSliderValue(fMaxValue));
}


// #pragma mark -


void
DualSlider::SetLabel(const char* label)
{
	if (label != NULL)
		fLabel->setText(QString::fromUtf8(label));
}


void
DualSlider::SetValues(float min, float max)
{
	SetMinValue(min);
	SetMaxValue(max);
}


void
DualSlider::SetMinValue(float value)
{
	if (value < 0.0)
		value = 0.0;
	if (value > 1.0)
		value = 1.0;
	if (value != fMinValue) {
		fMinValue = value;

		fIgnoreMinSliderChange = true;
		fMinSlider->setValue(_ToSliderValue(fMinValue));
		fIgnoreMinSliderChange = false;

		_Invoke(fValueMessage);
	}
}


void
DualSlider::SetMaxValue(float value)
{
	if (value < 0.0)
		value = 0.0;
	if (value > 1.0)
		value = 1.0;
	if (value != fMaxValue) {
		fMaxValue = value;

		fIgnoreMaxSliderChange = true;
		fMaxSlider->setValue(_ToSliderValue(fMaxValue));
		fIgnoreMaxSliderChange = false;

		_Invoke(fValueMessage);
	}
}


void
DualSlider::SetEnabled(bool enable)
{
	setEnabled(enable);
}


bool
DualSlider::IsEnabled() const
{
	return isEnabled();
}


void
DualSlider::SetMinEnabled(bool enable, bool sendMessage)
{
	if (IsMinEnabled() != enable) {
		fCheckBox->setChecked(enable);
		fMinSlider->setEnabled(enable);
		if (sendMessage)
			_Invoke(fControlMessage);
	}
}


bool
DualSlider::IsMinEnabled() const
{
	return fMinSlider->isEnabledTo(const_cast<DualSlider*>(this));
}


void
DualSlider::SetMaxEnabled(bool enable)
{
	fMaxSlider->setEnabled(enable);
}


bool
DualSlider::IsMaxEnabled() const
{
	return fMaxSlider->isEnabledTo(const_cast<DualSlider*>(this));
}


void
DualSlider::SetPressureControlTip(const char* text)
{
	fCheckBox->setToolTip(QString::fromUtf8(text));
}

// #pragma mark -


void
DualSlider::_Invoke(BMessage* fromMessage)
{
	if (fromMessage == NULL)
		fromMessage = fValueMessage;
	if (fromMessage != NULL) {
#if 0
		BHandler* target = fTarget != NULL ? fTarget : Window();
		BLooper* looper;
		if (target != NULL && (looper = target->Looper())) {
			BMessage message(*fromMessage);
			message.AddPointer("be:source", (void*)this);
			message.AddInt64("be:when", system_time());
			if (fromMessage == fValueMessage) {
				message.AddFloat("min value", fMinValue);
				message.AddFloat("max value", fMaxValue);
			}
			if (fromMessage == fControlMessage)
				message.AddInt32("be:value", (int32)IsMinEnabled());
			looper->PostMessage(&message, target);
		}
#endif
// TODO:...
	}
}

// _ValueFor
float
DualSlider::_FromSliderValue(int sliderValue) const
{
	return (float)sliderValue / kMaxSliderValue;
}


int
DualSlider::_ToSliderValue(float value) const
{
	return int(value * kMaxSliderValue);
}


void
DualSlider::_MinValueChanged()
{
	if (fIgnoreMinSliderChange)
		return;

	if (!fDraggingMinSlider)
		fLastFactor = fMinValue != 0 ? fMaxValue / fMinValue : 1;

	fMinValue = _FromSliderValue(fMinSlider->value());
	if (fLastFactor < 1.0)
		SetMaxValue(fLastFactor * fMinValue);

}


void
DualSlider::_MaxValueChanged()
{
	if (fIgnoreMaxSliderChange)
		return;

	if (!fDraggingMaxSlider)
		fLastFactor = fMaxValue != 0 ? fMinValue / fMaxValue : 1;

	fMaxValue = _FromSliderValue(fMaxSlider->value());
	if (fLastFactor < 1.0)
		SetMinValue(fLastFactor * fMaxValue);
}


void
DualSlider::_MinSliderPressed()
{
	fDraggingMinSlider = true;
	fLastFactor = fMinValue != 0 ? fMaxValue / fMinValue : 1;
}


void
DualSlider::_MaxSliderPressed()
{
	fDraggingMaxSlider = true;
	fLastFactor = fMaxValue != 0 ? fMinValue / fMaxValue : 1;
}


void
DualSlider::_MinSliderReleased()
{
	fDraggingMinSlider = false;
}


void
DualSlider::_MaxSliderReleased()
{
	fDraggingMaxSlider = false;
}


void
DualSlider::_CheckBoxStateChanged()
{
	SetMinEnabled(fCheckBox->isChecked());
}
