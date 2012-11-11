#ifndef COLOR_PICKER_VIEW_PLATFORM_DELEGATE_H
#define COLOR_PICKER_VIEW_PLATFORM_DELEGATE_H


#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "ColorPickerView.h"
#include "PlatformSignalMessageAdapter.h"


class ColorPickerView::PlatformDelegate {
public:
	PlatformDelegate(ColorPickerView* view)
		:
		fView(view)
	{
	}

	PlatformDelegate()
	{
		for (int i = 0; i < 6; i++)
			fTextControlAdapters[i].DisconnectAll();
		fHexTextControlAdapter.DisconnectAll();
	}


	void Init(int32 selectedRadioButton)
	{
		QBoxLayout* mainLayout = new QHBoxLayout(fView);
		mainLayout->addWidget(fView->fColorField);
		mainLayout->addWidget(fView->fColorSlider);

		// right hand side layout
		QWidget* rightLayoutWidget = new QWidget;
		rightLayoutWidget->setSizePolicy(QSizePolicy::Fixed,
			QSizePolicy::Preferred);
		QBoxLayout* rightLayout = new QVBoxLayout(rightLayoutWidget);
		mainLayout->addWidget(rightLayoutWidget);
		rightLayout->setMargin(0);

		// color preview
		rightLayout->addWidget(fView->fColorPreview);

		// channel controls
		const char* const labels[6] = { "H", "S", "V", "R", "G", "B" };
		const char* const units[6] = { "°", "%", "%" };
		const int rangeMaxValues[6] = { 359, 100, 100, 255, 255, 255 };

		QGridLayout* gridLayout = new QGridLayout();
		rightLayout->addLayout(gridLayout);

		for (int i = 0; i < 2; i++) {
			for (int k = 0; k < 3; k++) {
				int row = i * 3 + k;
				fRadioButton[row]
					= new QRadioButton(QString::fromUtf8(labels[row]));
				PlatformSignalMessageAdapter* radioButtonAdapter
					= new PlatformSignalMessageAdapter(fRadioButton[row]);
				radioButtonAdapter->Connect(fRadioButton[row],
					SIGNAL(clicked()), fView, MSG_RADIOBUTTON + row);
				gridLayout->addWidget(fRadioButton[row], row, 0);

				fTextControl[row] = new QSpinBox;
				fTextControl[row]->setMaximum(rangeMaxValues[row]);
				fTextControlAdapters[row].Connect(fTextControl[row],
					SIGNAL(valueChanged(int)), fView, MSG_TEXTCONTROL + row);
				gridLayout->addWidget(fTextControl[row], row, 1);

				if (const char* unit = units[row]) {
					gridLayout->addWidget(
						new QLabel(QString::fromUtf8(unit)), row, 2);
				}
			}
		}

		fRadioButton[selectedRadioButton]->setChecked(true);

		// hex text control
		QBoxLayout* layout = new QHBoxLayout;
		rightLayout->addLayout(layout);
		layout->addWidget(new QLabel(QString::fromUtf8("#")));
		layout->addWidget(fHexTextControl = new QLineEdit);
		fHexTextControl->setText(QString::fromUtf8("000000"));
		fHexTextControl->setInputMask(QString::fromUtf8("HHHHHH"));
		fHexTextControlAdapter.Connect(fHexTextControl,
			SIGNAL(textEdited(QString)), fView, MSG_HEXTEXTCONTROL);
	}

	void Draw(PlatformDrawContext& drawContext)
	{
	}

	int TextControlValue(int32 index)
	{
		return fTextControl[index]->value();
	}

	// Returns whether the value needs to be set later, since it is currently
	// being edited by the user.
	bool SetTextControlValue(int32 index, int value)
	{
		if (fTextControl[index]->value() == value)
			return false;

		if (fTextControl[index]->hasFocus())
			return true;

		fTextControlAdapters[index].Suspend();
		fTextControl[index]->setValue(value);
		fTextControlAdapters[index].Resume();
		return false;
	}

	BString HexTextControlString() const
	{
		return fHexTextControl->text();
	}

	// Returns whether the value needs to be set later, since it is currently
	// being edited by the user.
	bool SetHexTextControlString(const BString& text)
	{
		QString qtText = text;
		if (fHexTextControl->text() == qtText)
			return false;

		if (fHexTextControl->hasFocus())
			return true;

		fHexTextControlAdapter.Suspend();
		fHexTextControl->setText(qtText);
		fHexTextControlAdapter.Resume();
		return false;
	}

private:
	ColorPickerView*	fView;
	QRadioButton*					fRadioButton[6];
	QSpinBox*						fTextControl[6];
	QLineEdit*						fHexTextControl;
	PlatformSignalMessageAdapter	fTextControlAdapters[6];
	PlatformSignalMessageAdapter	fHexTextControlAdapter;
};


#endif // COLOR_PICKER_VIEW_PLATFORM_DELEGATE_H
