#ifndef ICONBUTTON_H
#define ICONBUTTON_H


#include <Handler.h>
#include <Invoker.h>
#include <Rect.h>
#include <SupportDefs.h>

#include <QToolButton>


class BBitmap;


class IconButton : public PlatformWidgetHandler<QToolButton>, public BInvoker
{
	Q_OBJECT

public:
//	explicit					IconButton(QWidget* parent = NULL);
								IconButton(const char* name,
										   uint32 id,
										   const char* label = NULL,
										   BMessage* message = NULL,
										   BHandler* target = NULL);
								~IconButton();

	// BInvoker interface
	virtual	status_t			Invoke(BMessage* message = NULL);

	// IconButton
			bool				IsValid() const;

	virtual	int32				Value() const;
	virtual	void				SetValue(int32 value);

			void				SetPressed(bool pressed);
			uint32				ID() const
									{ return fID; }

			status_t			SetIcon(int32 resourceID, int32 size);
			status_t			SetIcon(const BBitmap* bitmap);

			void				TrimIcon(BRect bounds);

private slots:
			void				_CheckedChanged();

private:
			status_t			_MakeBitmaps(const BBitmap* bitmap);
			void				_DeleteBitmaps();

			void				_UpdateIcon();

private:
			int32				fID;
			BBitmap*			fNormalBitmap;
			QIcon*				fNormalIcon;
			QIcon*				fDisabledIcon;
			QIcon*				fClickedIcon;
			QIcon*				fDisabledClickedIcon;
};


#endif // ICONBUTTON_H
