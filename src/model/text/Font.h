/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */
#ifndef FONT_H
#define FONT_H

#include <String.h>

class Font {
public:
	enum ScriptLevel {
		NORMAL = 0, SUBSCRIPT, SUPERSCRIPT, LOW_SUPERSCRIPT, HIGH_SUBSCRIPT
	};

public:
	Font(const char* family, double size);
	Font(const char* family, const char* style, double size);
	Font(const char* family, const char* style, double size,
		ScriptLevel scriptLevel);
	Font(const Font& other);
	virtual ~Font();

	bool operator==(const Font& other) const;
	bool operator!=(const Font& other) const;

	Font& operator=(const Font& other);

	inline const char* getFamily() const
	{
		return fFamily;
	}

	inline const char* getStyle() const
	{
		return fStyle;
	}

	inline double getSize() const
	{
		return fSize;
	}

	inline ScriptLevel getScriptLevel() const
	{
		return fScriptLevel;
	}

	const char* getFontFilePath() const;

	// Setters return new Font instance with the changed property
	Font setFamilyAndStyle(const char* family, const char* style) const;
	Font setSize(double size) const;
	Font setScriptLevel(ScriptLevel scriptLevel) const;

private:
	BString				fFamily;
	BString				fStyle;
	mutable BString		fFontFilePath;
	double				fSize;
	ScriptLevel			fScriptLevel;
};

#endif // FONT_H
