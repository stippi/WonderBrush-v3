#ifndef BRUSH_TOOL_CONFIG_VIEW_H
#define BRUSH_TOOL_CONFIG_VIEW_H


#include "ToolConfigView.h"


namespace Ui {
class BrushToolConfigView;
}


class BrushToolConfigView : public ToolConfigView
{
	Q_OBJECT
	
public:
								BrushToolConfigView(::Tool* tool);
	virtual						~BrushToolConfigView();

	// ToolConfigView interface
	virtual	void				UpdateStrings();
	virtual	void				SetActive(bool active);
	virtual	void				SetEnabled(bool enable);

	// BView interface
	virtual	void				AttachedToWindow();
	virtual	void				MessageReceived(BMessage* message);

private:
			void				_SetMinMaxOption(const BMessage* message,
									uint32 minOption, uint32 maxOption);

private slots:
			void				_SubpixelsChanged();
			void				_SolidChanged();
			void				_TiltChanged();

private:
			Ui::BrushToolConfigView* fUi;
};


#endif // BRUSH_TOOL_CONFIG_VIEW_H
