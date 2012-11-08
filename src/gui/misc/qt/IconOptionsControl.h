#ifndef ICONOPTIONSCONTROL_H
#define ICONOPTIONSCONTROL_H


#include <Control.h>


class QBoxLayout;

class IconButton;


class IconOptionsControl : public BControl {
public:
								IconOptionsControl(QWidget* parent);
								IconOptionsControl(const char* name = NULL,
									const char* label = NULL,
									BMessage* message = NULL,
									BHandler* target = NULL,
									enum orientation orientation
										= B_HORIZONTAL);

			void				AddOption(IconButton* icon);

	virtual	void				AllAttached();
	virtual	void				MessageReceived(BMessage* message);

	virtual	void				SetValue(int32 value);
	virtual	void				SetEnabled(bool enable);

private:
			void				_Init(enum orientation orientation);

			IconButton*			_FindIcon(int32 index) const;

private:
			QWidget*			fIconGroup;
			QBoxLayout*			fIconGroupLayout;
			QList<IconButton*>	fIconButtons;
			BHandler*			fTargetCache;
};


#endif // ICONOPTIONSCONTROL_H
