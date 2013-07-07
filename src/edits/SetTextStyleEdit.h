/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef SET_TEXT_STYLE_EDIT_H
#define SET_TEXT_STYLE_EDIT_H

#include <String.h>

#include "UndoableEdit.h"
#include "StyleRun.h"
#include "StyleRunList.h"
#include "Text.h"

class SetTextStyleEdit : public UndoableEdit {
private:
	class CharacterStyleModifier {
	public:
		virtual CharacterStyle ModifyStyle(const CharacterStyle& style)
			const = 0;
	};

	class FontSizeModifier : public CharacterStyleModifier {
	public:
		FontSizeModifier(double fontSize)
			: fFontSize(fontSize)
		{
		}
		virtual CharacterStyle ModifyStyle(const CharacterStyle& style) const
		{
			return style.SetFont(style.GetFont().setSize(fFontSize));
		}
	private:
		double	fFontSize;
	};

	class FontFamilyAndStyleModifier : public CharacterStyleModifier {
	public:
		FontFamilyAndStyleModifier(const char* family, const char* style)
			: fFamily(family)
			, fStyle(style)
		{
		}
		virtual CharacterStyle ModifyStyle(const CharacterStyle& style) const
		{
			return style.SetFont(style.GetFont().setFamilyAndStyle(
				fFamily.String(), fStyle.String()));
		}
	private:
		BString	fFamily;
		BString	fStyle;
	};

	class StyleModifier : public CharacterStyleModifier {
	public:
		StyleModifier(const StyleRef& styleRef)
			: fStyleRef(styleRef)
		{
		}
		virtual CharacterStyle ModifyStyle(const CharacterStyle& style) const
		{
			return style.SetStyle(fStyleRef);
		}
	private:
		StyleRef	fStyleRef;
	};

	class GlyphSpacingModifier : public CharacterStyleModifier {
	public:
		GlyphSpacingModifier(double spacing)
			: fGlyphSpacing(spacing)
		{
		}
		virtual CharacterStyle ModifyStyle(const CharacterStyle& style) const
		{
			return style.SetGlyphSpacing(fGlyphSpacing);
		}
	private:
		double	fGlyphSpacing;
	};

	class FauxWeightModifier : public CharacterStyleModifier {
	public:
		FauxWeightModifier(double fauxWeight)
			: fFauxWeight(fauxWeight)
		{
		}
		virtual CharacterStyle ModifyStyle(const CharacterStyle& style) const
		{
			return style.SetFauxWeight(fFauxWeight);
		}
	private:
		double	fFauxWeight;
	};

	class FauxItalicModifier : public CharacterStyleModifier {
	public:
		FauxItalicModifier(double fauxItalic)
			: fFauxItalic(fauxItalic)
		{
		}
		virtual CharacterStyle ModifyStyle(const CharacterStyle& style) const
		{
			return style.SetFauxItalic(fFauxItalic);
		}
	private:
		double	fFauxItalic;
	};


public:
	SetTextStyleEdit(Text* text, int32 textOffset, int32 length,
		const rgb_color& color)
		: UndoableEdit()
		, fCommandName("Change text color")
	{
		_Init(text, textOffset, length);

		::Style* style = new(std::nothrow) ::Style();
		if (style == NULL)
			return;

		style->SetFillPaint(Paint(color));

		_ModifyStyles(StyleModifier(StyleRef(style, true)));
	}

	SetTextStyleEdit(Text* text, int32 textOffset, int32 length,
		double fontSize)
		: UndoableEdit()
		, fCommandName("Change text size")
	{
		_Init(text, textOffset, length);
		_ModifyStyles(FontSizeModifier(fontSize));
	}

	SetTextStyleEdit(Text* text, int32 textOffset, int32 length,
		const char* family, const char* style)
		: UndoableEdit()
		, fCommandName("Change text font")
	{
		_Init(text, textOffset, length);
		_ModifyStyles(FontFamilyAndStyleModifier(family, style));
	}

	SetTextStyleEdit(Text* text, int32 textOffset, int32 length)
		: UndoableEdit()
		, fCommandName("Change text font")
	{
		_Init(text, textOffset, length);
	}

	virtual ~SetTextStyleEdit()
	{
		delete fOldStyles;
		delete fNewStyles;
	}

	void SetGlyphSpacing(double spacing)
	{
		_ModifyStyles(GlyphSpacingModifier(spacing));
	}

	void SetFauxWeight(double fauxWeight)
	{
		_ModifyStyles(FauxWeightModifier(fauxWeight));
	}

	void SetFauxItalic(double fauxItalic)
	{
		_ModifyStyles(FauxItalicModifier(fauxItalic));
	}

	virtual	status_t InitCheck()
	{
		return fText.Get() != NULL && fOldStyles != NULL && fNewStyles != NULL
			? B_OK : B_ERROR;
	}

	virtual	status_t Perform(EditContext& context)
	{
		fText->ReplaceStyles(fOffset, fString.CountChars(), *fNewStyles);
		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		fText->ReplaceStyles(fOffset, fString.CountChars(), *fOldStyles);
		return B_OK;
	}

	virtual void GetName(BString& name)
	{
		name << fCommandName;
	}

	virtual	bool CombineWithNext(const UndoableEdit* _next)
	{
		const SetTextStyleEdit* next
			= dynamic_cast<const SetTextStyleEdit*>(_next);

		if (next == NULL || next->fText != fText
			|| next->fTimeStamp - fTimeStamp > 500000
			|| next->fOldStyles == NULL
			|| next->fOffset != fOffset
			|| next->fCommandName != fCommandName
			|| next->fString != fString) {
			return false;
		}

		*fNewStyles = *(next->fNewStyles);
		fTimeStamp = next->fTimeStamp;

		return true;
	}

private:
	void _Init(Text* text, int32 offset, int32 length)
	{
		fNewStyles = NULL;
		if (text == NULL)
			return;

		fText.SetTo(text);
		fOffset = offset;
		fString = fText->GetSubString(offset, length);
		fOldStyles = fText->GetStyleRuns(offset, length);
		if (fOldStyles != NULL)
			fNewStyles = new(std::nothrow) StyleRunList(*fOldStyles);
	}

	void _ModifyStyles(const CharacterStyleModifier& modifier)
	{
		if (fOldStyles == NULL || fNewStyles == NULL)
			return;

		StyleRunList* newStyles = new(std::nothrow) StyleRunList();
		if (newStyles == NULL)
			return;

		int32 textOffset = 0;
		int32 runCount = fNewStyles->CountRuns();
		for (int32 i = 0; i < runCount; i++) {
			const StyleRun* run = fNewStyles->StyleRunAt(i);

			CharacterStyle* characterStyle = new(std::nothrow) CharacterStyle(
				modifier.ModifyStyle(*(run->GetStyle().Get())));

			if (characterStyle == NULL) {
				delete newStyles;
				return;
			}

			CharacterStyleRef styleRef(characterStyle, true);

			StyleRun replacementRun(styleRef);
			replacementRun.SetLength(run->GetLength());

			if (!newStyles->Append(replacementRun)) {
				delete newStyles;
				return;
			}

			styleRef.Detach();
			textOffset += run->GetLength();
		}

		// Swap the StyleRunLists
		delete fNewStyles;
		fNewStyles = newStyles;
	}

private:
			Reference<Text>		fText;
			int32				fOffset;
			BString				fString;
			StyleRunList*		fOldStyles;
			StyleRunList*		fNewStyles;
			BString				fCommandName;
};

#endif // SET_TEXT_STYLE_EDIT_H
