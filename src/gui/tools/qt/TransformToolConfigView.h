#ifndef TRANSFORM_TOOL_CONFIG_VIEW_H
#define TRANSFORM_TOOL_CONFIG_VIEW_H


#include "ToolConfigView.h"

#include <QDoubleSpinBox>


namespace Ui {
	class TransformToolConfigView;
}

class TransformToolConfigView : public ToolConfigView {
	Q_OBJECT
	
public:
								TransformToolConfigView(::Tool* tool);
								~TransformToolConfigView();

	// ToolConfigView interface
	virtual	void				UpdateStrings();
	virtual	void				SetActive(bool active);
	virtual	void				SetEnabled(bool enable);

	// BView interface
	virtual	void				MessageReceived(BMessage* message);

private:
			void				_SetValue(QDoubleSpinBox* control,
									float value) const;

private slots:
			void				_TranslationXChanged();
			void				_TranslationYChanged();
			void				_RotationChanged();
			void				_ScaleXChanged();
			void				_ScaleYChanged();
			void				_SubpixelsChanged();

private:
			Ui::TransformToolConfigView* fUi;

			bool				fNotificationsEnabled;
};


#endif // TRANSFORM_TOOL_CONFIG_VIEW_H
