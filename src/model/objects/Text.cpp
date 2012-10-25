/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "Text.h"

#include "Font.h"
#include "FontCache.h"
#include "RenderEngine.h"
#include "TextSnapshot.h"
#include "ui_defines.h"

typedef Reference<Style>	StyleRef;

class Text::CharacterStyle : public Referenceable {
public:
	CharacterStyle(const Font& font, const StyleRef& style)
		: fFont(font)
		, fStyle(style)
	{
	}

	const Font& GetFont() const
	{
		return fFont;
	}

	const StyleRef& GetStyle() const
	{
		return fStyle;
	}
	
	bool operator==(const CharacterStyle& other) const
	{
		return fFont == other.fFont && *(fStyle.Get()) == *(other.fStyle.Get());
	}

	bool operator!=(const CharacterStyle& other) const
	{
		return *this != other;
	}
	
private:
	Font				fFont;
	StyleRef			fStyle;
};

// #pragma mark -

class Text::StyleRun {
public:
	StyleRun(const CharacterStyleRef& characterStyle)
		: fCharacterStyle(characterStyle)
		, fLength(0)
	{
	}

	StyleRun(const StyleRun& other)
		: fCharacterStyle(other.fCharacterStyle)
		, fLength(other.fLength)
	{
	}

	const CharacterStyleRef& GetStyle() const
	{
		return fCharacterStyle;
	}

	int32 GetLength() const
	{
		return fLength;
	}

	void SetLength(int32 length)
	{
		fLength = length;
	}

	bool IsSameStyle(const StyleRun& other) const
	{
		return *fCharacterStyle.Get() == *other.fCharacterStyle.Get();
	}

private:
	CharacterStyleRef	fCharacterStyle;
	int32				fLength;
};

// #pragma mark -

class Text::StyleRunList {
public:
	StyleRunList()
		: fRuns(4)
	{
	}

	~StyleRunList()
	{
		MakeEmpty();
	}

	void MakeEmpty()
	{
		for (int32 i = fRuns.CountItems(); i >= 0; i--)
			delete (StyleRun*)fRuns.ItemAtFast(i);
		fRuns.MakeEmpty();
	}

	bool Append(const StyleRun& runToAppend)
	{
		if (fRuns.CountItems() > 0) {
			StyleRun* run
				= (StyleRun*)fRuns.ItemAtFast(fRuns.CountItems() - 1);
			if (run->IsSameStyle(runToAppend)) {
				run->SetLength(run->GetLength() + runToAppend.GetLength());
				return true;
			}
		}
		
		StyleRun* inserted = new(std::nothrow) StyleRun(runToAppend);
		if (inserted == NULL || !fRuns.AddItem(inserted)) {
			delete inserted;
			return false;
		}
		
		return true;
	}

	bool Insert(int32 textOffset, const StyleRun& runToInsert)
	{
//printf("StyleRunList::Insert(%ld, StyleRun(%s, %.1f, %ld))\n",
//	textOffset, runToInsert.GetStyle()->GetFont().getName(),
//	runToInsert.GetStyle()->GetFont().getSize(), runToInsert.GetLength());
		int32 startOffset = 0;
		int32 styleCount = fRuns.CountItems();
		for (int32 i = 0; i < styleCount; i++) {
			StyleRun* run = (StyleRun*)fRuns.ItemAtFast(i);
			if (startOffset == textOffset
				|| startOffset + run->GetLength() == textOffset) {
				if (run->IsSameStyle(runToInsert)) {
					// Runs equal, just increase length of existing run
					run->SetLength(run->GetLength() + runToInsert.GetLength());
					return true;
				}
				// Insert before or after run at this index
				StyleRun* inserted = new(std::nothrow) StyleRun(runToInsert);
				if (startOffset != textOffset)
					i++;
				if (inserted == NULL || !fRuns.AddItem(inserted, i)) {
					delete inserted;
					return false;
				}
				return true;
			}
			
			if (startOffset + run->GetLength() > textOffset) {
				// Insert here
				if (run->IsSameStyle(runToInsert)) {
					// Runs equal, just increase length of existing run
					run->SetLength(run->GetLength() + runToInsert.GetLength());
					return true;
				}
				// Split run at this index
				StyleRun* remaining = new(std::nothrow) StyleRun(*run);
				if (remaining == NULL || !fRuns.AddItem(remaining, i + 1)) {
					delete remaining;
					return false;
				}
				// Insert new style
				StyleRun* inserted = new(std::nothrow) StyleRun(runToInsert);
				if (inserted == NULL || !fRuns.AddItem(inserted, i + 1)) {
					delete (StyleRun*)fRuns.RemoveItem(i + 1);
					delete inserted;
					return false;
				}
				// Success, update lengths of the two split runs
				int32 previousLength = run->GetLength();
				int32 newLength = textOffset - startOffset;
				run->SetLength(newLength);
				remaining->SetLength(previousLength - newLength);
				return true;
			}
			
			startOffset += run->GetLength();
		}
		
		return Append(runToInsert);
	}

	void Remove(int32 textOffset, int32 length)
	{
		if (textOffset < 0) {
			length += textOffset;
			textOffset = 0;
		}
		if (length <= 0)
			return;

		bool removedRuns = false;

		int32 startOffset = 0;
		for (int32 i = 0; i < fRuns.CountItems(); i++) {
			StyleRun* run = (StyleRun*)fRuns.ItemAtFast(i);
			if (startOffset == textOffset
				|| startOffset + run->GetLength() > textOffset) {
					
				int32 newRunLength = 0;
				if (startOffset < textOffset)
					newRunLength += textOffset - startOffset;
				
				if (textOffset + length < startOffset + run->GetLength())
					newRunLength += (startOffset + run->GetLength())
						- (textOffset + length);
					
				if (newRunLength == 0) {
					length -= run->GetLength();
					// Remove this run altogether
					fRuns.RemoveItem(i);
					i--;
					delete run;
					removedRuns = true;
					continue;
				}
				
				length -= run->GetLength() - newRunLength;
				run->SetLength(newRunLength);
				if (length < 0)
					debugger("Removed too much in StyleRunList::Remove()");
				
				if (length == 0)
					break;
			}
			
			startOffset += run->GetLength();
		}
		
		if (removedRuns) {
			// Removing runs may have caused equal runs to attach
			_MergeEqualRuns();
		}
	}

	const StyleRun* FindStyleRun(int32 textOffset) const
	{
		if (textOffset < 0)
			return NULL;
		
		int32 startOffset = 0;
		int32 styleCount = fRuns.CountItems();
		for (int32 i = 0; i < styleCount; i++) {
			StyleRun* run = (StyleRun*)fRuns.ItemAtFast(i);
			startOffset += run->GetLength();
			if (startOffset > textOffset)
				return run;
		}
		
		return (StyleRun*)fRuns.LastItem();
	}

	const StyleRun* StyleRunAt(int32 index) const
	{
		return (StyleRun*)fRuns.ItemAt(index);
	}

	const int32 CountRuns() const
	{
		return fRuns.CountItems();
	}

private:
	
	void _MergeEqualRuns()
	{
		for (int32 i = 0; i < fRuns.CountItems() - 1; i++) {
			StyleRun* runA = (StyleRun*)fRuns.ItemAtFast(i);
			StyleRun* runB = (StyleRun*)fRuns.ItemAtFast(i + 1);
			if (runA->IsSameStyle(*runB)) {
				fRuns.RemoveItem(i + 1);
				runA->SetLength(runA->GetLength() + runB->GetLength());
				delete runB;
				i--;
			}
		}
	}

	BList		fRuns;
};

// #pragma mark -

// constructor
Text::Text(const rgb_color& color)
	: Styleable(color)
	, fText()
	, fCharCount(0)
	, fTextLayout(FontCache::getInstance())
	, fStyleRuns(new(std::nothrow) StyleRunList())
{
	InitBounds();
}

// destructor
Text::~Text()
{
	delete fStyleRuns;
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Text::Snapshot() const
{
	return new TextSnapshot(this);
}

// DefaultName
const char*
Text::DefaultName() const
{
	return "Text";
}

// HitTest
bool
Text::HitTest(const BPoint& canvasPoint)
{
	RenderEngine engine(Transformation());
	return engine.HitTest(Bounds(), canvasPoint);
}

// Bounds
BRect
Text::Bounds()
{
	BRect bounds(0.0f, 0.0f, fTextLayout.getActualWidth(),
		fTextLayout.getHeight());
	Style()->ExtendBounds(bounds);
	return bounds;
}

// #pragma mark -

// SetWidth
void
Text::SetWidth(double width)
{
	if (width == fTextLayout.getWidth())
		return;

	fTextLayout.setWidth(width);

	NotifyAndUpdate();
}

// Width
double
Text::Width()
{
	return fTextLayout.getWidth();
}

// SetAlignment
void
Text::SetAlignment(uint32 alignment)
{
	if (alignment == fTextLayout.getAlignment())
		return;

	fTextLayout.setAlignment(alignment);

	NotifyAndUpdate();
}

// SetJustify
void
Text::SetJustify(bool justify)
{
	if (justify == fTextLayout.getJustify())
		return;

	fTextLayout.setJustify(justify);

	NotifyAndUpdate();
}

// SetText
void
Text::SetText(const char* utf8String, const char* fontFilePath, double size,
	rgb_color color)
{
	::Style* style = new(std::nothrow) ::Style();
	if (style == NULL)
		return;
	
	style->SetFillPaint(Paint(color));
	
	StyleRef styleRef(style, true);
	SetText(utf8String, fontFilePath, size, styleRef);
}

// SetText
void
Text::SetText(const char* utf8String, const char* fontFilePath, double size,
	const StyleRef& style)
{
	fText = "";
	fStyleRuns->MakeEmpty();

	Insert(0, utf8String, fontFilePath, size, style);
}

// GetText
const char*
Text::GetText() const
{
	return fText.String();
}

// GetCharCount
int32
Text::GetCharCount() const
{
	return fCharCount;
}

// Insert
void
Text::Insert(int32 textOffset, const char* utf8String,
	const char* fontFilePath, double size, const StyleRef& style)
{
	if (textOffset < 0 || textOffset > fText.CountChars())
		return;
	
	CharacterStyle* characterStyle = new(std::nothrow) CharacterStyle(
		Font(fontFilePath, size), style);

	if (characterStyle == NULL)
		return;

	CharacterStyleRef styleRef(characterStyle);

	BString text(utf8String);
	int32 charCount = text.CountChars();

	StyleRun styleRun(styleRef);
	styleRun.SetLength(charCount);

	if (!fStyleRuns->Insert(textOffset, styleRun))
		return;

	fText.InsertChars(text, textOffset);
	fCharCount += charCount;

	styleRef.Detach();

	_UpdateLayout();
}

// Remove
void
Text::Remove(int32 textOffset, int32 length)
{
	if (textOffset < 0 || textOffset + length > fText.CountChars())
		return;

	fText.RemoveChars(textOffset, length);
	fStyleRuns->Remove(textOffset, length);
	fCharCount -= length;

	_UpdateLayout();
}

// SetStyle
void
Text::SetStyle(int32 textOffset, int32 length,
	const char* utf8String, const char* fontFilePath, double size,
	const StyleRef& style)
{
	if (textOffset < 0 || textOffset + length > fText.CountChars())
		return;

	CharacterStyle* characterStyle = new(std::nothrow) CharacterStyle(
		Font(fontFilePath, size), style);

	if (characterStyle == NULL)
		return;

	CharacterStyleRef styleRef(characterStyle);

	StyleRun styleRun(styleRef);
	styleRun.SetLength(length);

	if (!fStyleRuns->Insert(textOffset, styleRun))
		return;

	fStyleRuns->Remove(textOffset + length, length);

	styleRef.Detach();

	_UpdateLayout();
}

// getTextLayout
TextLayout&
Text::getTextLayout()
{
	return fTextLayout;
}

// getTextLayout
const TextLayout&
Text::getTextLayout() const
{
	return fTextLayout;
}

// #pragma mark -

// _UpdateLayout
void
Text::_UpdateLayout()
{
//	printf("_UpdateLayout()\n");

	fTextLayout.clearStyleRuns();
	
	int32 start = 0;
	for (int32 i = 0; i < fStyleRuns->CountRuns(); i++) {
		const StyleRun* run = fStyleRuns->StyleRunAt(i);
		
		const CharacterStyleRef& characterStyle = run->GetStyle();
		
		const Font& font = characterStyle.Get()->GetFont();
		const StyleRef& style = characterStyle.Get()->GetStyle();
		Paint* paint = style.Get()->FillPaint();
		rgb_color color = paint->Color();
		
//		printf("  run (%d, %d, %d), length: %ld\n",
//			color.red, color.green, color.blue,
//			run->GetLength());

		fTextLayout.addStyleRun(
			start, font.getName(), font.getSize(), font.getStyle(),
			0.0, 0.0, 0.0,
			color.red, color.green, color.blue,
			255, 255, 255,
			false, 0, 0, 0,
			false, 0, 0, 0, 0
		);
		
		start += run->GetLength();
	}
	
//	printf("  chars: %ld, total run length: %ld\n",
//		fText.CountChars(), start);
	if (fText.CountChars() != start)
		debugger("Text::_UpdateLayout() - StyleRunList invalid!");
	
	fTextLayout.setText(fText.String());

	NotifyAndUpdate();
}

