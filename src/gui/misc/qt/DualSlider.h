/*
 * Copyright 2002-2010 Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2012 Ingo Weinhold <ingo_weinhold@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef DUAL_SLIDER_H
#define DUAL_SLIDER_H


#include <Rect.h>
#include <String.h>
#include <View.h>

#include <QWidget>


class BHandler;
class BMessage;

class QCheckBox;
class QLabel;
class QSlider;


class DualSlider : public BView {
	Q_OBJECT

public:
								DualSlider(QWidget* parent = NULL,
									const char* label = NULL,
									BMessage* valueMessage = NULL,
									BMessage* controlMessage = NULL,
									BHandler* target = NULL,
									float minValue = 0.0f,
									float maxValue = 1.0f);
	virtual						~DualSlider();

			void				Init(const char* label = NULL,
									BMessage* valueMessage = NULL,
									BMessage* controlMessage = NULL,
									BHandler* target = NULL,
									float minValue = 0.0f,
									float maxValue = 1.0f);

	// DualSlider
			void				SetLabel(const char* label);

			void				SetValues(float min, float max);

			void				SetMinValue(float value);
	inline	float				MinValue() const
									{ return fMinValue; }

			void				SetMaxValue(float value);
	inline	float				MaxValue() const
									{ return fMaxValue; }

			void				SetEnabled(bool enable);
			bool				IsEnabled() const;

			void				SetMinEnabled(bool enable, 
									bool sendMessage = false);
			bool				IsMinEnabled() const;

			void				SetMaxEnabled(bool enable);
			bool				IsMaxEnabled() const;

			void				SetPressureControlTip(const char* text);

private:
			void				_Invoke(BMessage* message);

			float				_FromSliderValue(int sliderValue) const;
			int					_ToSliderValue(float value) const;

private slots:
			void				_MinValueChanged();
			void				_MaxValueChanged();
			void				_MinSliderPressed();
			void				_MaxSliderPressed();
			void				_MinSliderReleased();
			void				_MaxSliderReleased();
			void				_CheckBoxStateChanged();

private:
			QLabel*				fLabel;
			QCheckBox*			fCheckBox;
			QSlider*			fMinSlider;
			QSlider*			fMaxSlider;
			bool				fDraggingMinSlider;
			bool				fDraggingMaxSlider;
			bool				fIgnoreMinSliderChange;
			bool				fIgnoreMaxSliderChange;

			float				fMinValue;
			float				fMaxValue;
			float				fLastFactor;

			BMessage*			fValueMessage;
			BMessage*			fControlMessage;
			BHandler*			fTarget;
};

#endif // DUAL_SLIDER_H
