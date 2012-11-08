#ifndef FONT_POPUP_H
#define FONT_POPUP_H


#include <Invoker.h>
#include <String.h>

#include <QWidget>


class BMessage;


namespace Ui {
	class FontPopup;
}


class FontPopup : public QWidget, public BInvoker {
	Q_OBJECT
	
public:
	explicit					FontPopup(QWidget* parent = 0);
								~FontPopup();

			void				AddFont(const BString& fontFamily,
									const BString& fontStyle,
									BMessage* message = NULL);
			void				MakeEmpty();

			void				SetFamilyAndStyle(const char* family,
									const char* style);

private:
			struct Style;
			struct Family;
			struct FamilyListModel;
			struct StyleListModel;

private:
			Family*				_FindFamily(const BString& name) const;
			Family*				_CurrentFamily() const;
			Style*				_CurrentStyle() const;

private slots:
			void				_FamilyChanged();
			void				_StyleChanged();

private:
			Ui::FontPopup*		fUi;
			FamilyListModel*	fFamilyListModel;
			StyleListModel*		fStyleListModel;
			bool				fNotificationsEnabled;
};


#endif // FONT_POPUP_H
