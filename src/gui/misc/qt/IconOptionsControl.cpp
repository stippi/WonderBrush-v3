#include "IconOptionsControl.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "IconButton.h"


IconOptionsControl::IconOptionsControl(QWidget* parent)
	:
	BControl(NULL, NULL, NULL, 0),
	fTargetCache(NULL)
{
	setParent(parent);
	_Init(B_HORIZONTAL);
}


IconOptionsControl::IconOptionsControl(const char* name, const char* label,
	BMessage* message, BHandler* target, enum orientation orientation)
	:
	BControl(name, label, message, 0),
	fTargetCache(target)
{
	_Init(orientation);
}


void
IconOptionsControl::AddOption(IconButton* icon)
{
	if (icon == NULL)
		return;

	icon->SetRadioMode(true);

	// first icon added, mark it
	icon->SetPressed(!_FindIcon(0));

	fIconButtons.append(icon);
	fIconGroupLayout->addWidget(icon);
	icon->SetTarget(this);
}


void
IconOptionsControl::AllAttached()
{
	for (int32 i = 0; IconButton* button = _FindIcon(i); i++)
		button->SetTarget(this);
	if (fTargetCache != NULL)
		SetTarget(fTargetCache);
}


void
IconOptionsControl::MessageReceived(BMessage* message)
{
	// catch a message from the attached IconButtons to
	// handle switching the pressed icon
	BHandler* source;
	if (message->FindPointer("be:source", (void**)&source) >= B_OK) {
		if (IconButton* sourceIcon = dynamic_cast<IconButton*>(source)) {
			for (int32 i = 0; IconButton* button = _FindIcon(i); i++) {
				if (button == sourceIcon) {
					SetValue(i);
					break;
				}
			}
			// forward the message
			Invoke(message);
			return;
		}
	}
	BControl::MessageReceived(message);
}


void
IconOptionsControl::SetValue(int32 value)
{
	if (IconButton* valueButton = _FindIcon(value)) {
		for (int32 i = 0; IconButton* button = _FindIcon(i); i++) {
			button->SetPressed(button == valueButton);
		}
	}
	BControl::SetValueNoUpdate(value);
}


void
IconOptionsControl::SetEnabled(bool enable)
{
	for (int32 i = 0; IconButton* button = _FindIcon(i); i++) {
		button->setEnabled(enable);
	}
	BControl::SetEnabled(enable);
	Invalidate();
}


void
IconOptionsControl::_Init(enum orientation orientation)
{
	QBoxLayout* layout = new QHBoxLayout(this);

	fIconGroup = new(std::nothrow) QWidget(this);
	layout->addWidget(fIconGroup);

	if (orientation == B_HORIZONTAL)
		fIconGroupLayout = new(std::nothrow) QHBoxLayout(fIconGroup);
	else
		fIconGroupLayout = new(std::nothrow) QVBoxLayout(fIconGroup);
}


IconButton*
IconOptionsControl::_FindIcon(int32 index) const
{
	return fIconButtons.value(index, NULL);
}
