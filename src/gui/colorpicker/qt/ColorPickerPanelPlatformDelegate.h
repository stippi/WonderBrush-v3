#ifndef COLOR_PICKER_PANEL_PLATFORM_DELEGATE_H
#define COLOR_PICKER_PANEL_PLATFORM_DELEGATE_H


#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "ColorPickerPanel.h"
#include "ColorPickerView.h"
#include "PlatformSignalMessageAdapter.h"


class ColorPickerPanel::PlatformDelegate {
public:
	PlatformDelegate(ColorPickerPanel* panel)
		:
		fPanel(panel),
		fOkSignalAdapter(),
		fCancelSignalAdapter()
	{
	}

	void Init()
	{
		QWidget* mainWidget = new QWidget();
		fPanel->setCentralWidget(mainWidget);
		QVBoxLayout* layout = new QVBoxLayout(mainWidget);

		layout->addWidget(fPanel->fColorPickerView);

		QDialogButtonBox* buttonBox = new QDialogButtonBox(
			QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		layout->addWidget(buttonBox);

		fOkSignalAdapter.Connect(buttonBox, SIGNAL(accepted()), fPanel,
			MSG_DONE);
		fCancelSignalAdapter.Connect(buttonBox, SIGNAL(rejected()), fPanel,
			MSG_CANCEL);
		fCancelSignalAdapter.AddSource(fPanel, SIGNAL(PlatformWindowClosing()));
	}

private:
	ColorPickerPanel*				fPanel;
	PlatformSignalMessageAdapter	fOkSignalAdapter;
	PlatformSignalMessageAdapter	fCancelSignalAdapter;
};


#endif // COLOR_PICKER_PANEL_PLATFORM_DELEGATE_H
