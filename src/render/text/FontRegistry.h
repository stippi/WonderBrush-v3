/*
 * Copyright 2006-2012, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef FONT_REGISTRY_H
#define FONT_REGISTRY_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <Entry.h>
#include <Looper.h>
#include <String.h>

class BDirectory;
class BMenu;

enum {
	MSG_FONTS_CHANGED	= 'fnch',
};

enum {
	FONTS_CHANGED		= 0x01,
};

class FontRegistry : public BLooper {
public:
								FontRegistry();
	virtual						~FontRegistry();

								// BLooper
	virtual	void				MessageReceived(BMessage* message);

								// FontRegistry
	static	void				CreateDefault();
	static	void				DeleteDefault();
	static	FontRegistry*		Default();


			bool				AddFontDirectory(const char* path);
			void				Scan();

								// lock the object!
	virtual	const char*			FontFileAt(int32 index) const;
	virtual	const char*			FontFileFor(const char* family,
									const char* style) const;
	virtual	int32				IndexFor(const char* family,
									const char* style) const;

	virtual	const char*			FamilyFor(const char* fontFile) const;
	virtual	const char*			StyleFor(const char* fontFile) const;

	virtual	const char*			FullFamilyFor(const char* fontFile) const;
	virtual	const char*			PostScriptNameFor(const char* fontFile) const;

	virtual	int32				CountFontFiles() const;
			bool				GetFontAt(int32 index,
									char* family, char* style) const;

//	virtual	void				PopulateMenu(BMenu* menu, bool subMenus,
//											 const char* markedFamily,
//											 const char* markedStyle);

			FT_Library			FreeTypeLibrary() const
									{ return fLibrary; }

private:
	struct font_file {
		char*			family_name;
		char*			style_name;
		char*			full_family_name;
		char*			ps_name;
		BString			path;
	};

			void				_MakeEmpty();
	static	int32				_UpdateThreadEntry(void* cookie);
			void				_Update(BDirectory* fontFolder);
			void				_AddFont(const BEntry& entry);
			void				_ExtractFontNames(FT_Face face,
									font_file* fontFile,
									int32 nameCount) const;
			void				_ImproveStyleNameFromPostScriptName(
									font_file* fontFile) const;

			void				_DeleteFontFile(font_file* fontFile) const;

			font_file*			_FontFileFor(const char* ref) const;

private:
			FT_Library			fLibrary;			// the FreeType library
			BList				fFontDirectories;
			BList				fFontFiles;
	static	FontRegistry*		sDefaultRegistry;
};

#endif // FONT_REGISTRY_H
