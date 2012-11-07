#include "FontPopup.h"
#include "ui_FontPopup.h"

#include <Message.h>

#include "ListModel.h"


struct FontPopup::Style {
public:
	Style(const BString& name, BMessage* message)
		:
		fName(name),
		fMessage(message)
	{
	}

	~Style()
	{
		delete fMessage;
	}

	const BString& Name() const
	{
		return fName;
	}

	BMessage* Message() const
	{
		return fMessage;
	}

private:
	BString				fName;
	BMessage*			fMessage;
};


struct FontPopup::Family {
	Family(const BString& name)
		:
		fName(name)
	{
	}

	~Family()
	{
		foreach (Style* style, fStyles)
			delete style;
	}

	const BString& Name() const
	{
		return fName;
	}

	void AddStyle(const BString& name, BMessage* message)
	{
		fStyles.append(new Style(name, message));
	}

	const QList<Style*>& Styles() const
	{
		return fStyles;
	}

	Style* FindStyle(const char* name) const
	{
		foreach (Style* style, fStyles) {
			if (style->Name() == name)
				return style;
		}
		return NULL;
	}

private:
	BString			fName;
	QList<Style*>	fStyles;
};


struct FontPopup::FamilyListModel : public ListModel<Family*> {
	FamilyListModel(QObject* parent = NULL)
		:
		ListModel<Family*>(parent)
	{
	}

	virtual QString ItemText(Family* const& item) const
	{
		return item->Name().ToQString();
	}
};


struct FontPopup::StyleListModel : public ListModel<Style*> {
	StyleListModel(QObject* parent = NULL)
		:
		ListModel<Style*>(parent)
	{
	}

	virtual QString ItemText(Style* const& item) const
	{
		return item->Name().ToQString();
	}
};


FontPopup::FontPopup(QWidget* parent)
	:
	QWidget(parent),
	fUi(new Ui::FontPopup),
	fFamilyListModel(new FamilyListModel(this)),
	fStyleListModel(new StyleListModel(this)),
	fNotificationsEnabled(true)
{
	fUi->setupUi(this);

	fUi->familyComboBox->setModel(fFamilyListModel);
	fUi->styleComboBox->setModel(fStyleListModel);

	connect(fUi->familyComboBox, SIGNAL(currentIndexChanged(int)),
		SLOT(_FamilyChanged()));
	connect(fUi->styleComboBox, SIGNAL(currentIndexChanged(int)),
		SLOT(_StyleChanged()));
}


FontPopup::~FontPopup()
{
	MakeEmpty();

	delete fUi;
}


void
FontPopup::AddFont(const BString& fontFamily, const BString& fontStyle,
	BMessage* message)
{
	Family* family = _FindFamily(fontFamily);
	if (family == NULL) {
		family = new Family(fontFamily);
		fFamilyListModel->AppendItem(family);
	}

	family->AddStyle(fontStyle, message);
}

void
FontPopup::MakeEmpty()
{
	foreach (Family* family, fFamilyListModel->Items())
		delete family;

	fStyleListModel->MakeEmpty();
	fFamilyListModel->MakeEmpty();
}


void
FontPopup::SetFamilyAndStyle(const char* familyName, const char* styleName)
{
	Family* family = _FindFamily(familyName);
	Style* style = family != NULL ? family->FindStyle(styleName) : NULL;
	if (style != NULL) {
		fNotificationsEnabled = false;
		fUi->familyComboBox->setCurrentIndex(fFamilyListModel->IndexOf(family));
		fUi->styleComboBox->setCurrentIndex(fStyleListModel->IndexOf(style));
		fNotificationsEnabled = true;
	}
}


FontPopup::Family*
FontPopup::_FindFamily(const BString& name) const
{
	foreach (Family* family, fFamilyListModel->Items()) {
		if (family->Name() == name)
			return family;
	}

	return NULL;
}


FontPopup::Family*
FontPopup::_CurrentFamily() const
{
	return fFamilyListModel->ItemAt(fUi->familyComboBox->currentIndex());
}


FontPopup::Style*
FontPopup::_CurrentStyle() const
{
	return fStyleListModel->ItemAt(fUi->styleComboBox->currentIndex());
}


void
FontPopup::_FamilyChanged()
{
	if (Family* family = _CurrentFamily()) {
		fStyleListModel->SetItems(family->Styles());
		fUi->styleComboBox->setCurrentIndex(0);
	} else
		fStyleListModel->MakeEmpty();
}


void
FontPopup::_StyleChanged()
{
	if (!fNotificationsEnabled)
		return;

	Style* style = _CurrentStyle();
	if (style != NULL)
		Invoke(style->Message());
}
