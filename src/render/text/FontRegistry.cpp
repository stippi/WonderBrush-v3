/*
 * Copyright 2006-2012, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "FontRegistry.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <ft2build.h>
#include FT_SFNT_NAMES_H
#include <freetype/ttnameid.h>

#include <FindDirectory.h>
#include <Directory.h>
//#include <Menu.h>
//#include <MenuItem.h>
#include <Path.h>
#include <String.h>
#include <UTF8.h>

//#include "common.h"

//#include "FontPopup.h"

// static default instance
FontRegistry*
FontRegistry::sDefaultRegistry = NULL;

static const char* threadName = "font scanner";

enum {
	MSG_UPDATE			= 'updt',
};

// constructor
FontRegistry::FontRegistry()
	: BLooper(threadName, B_LOW_PRIORITY),
	  fLibrary(NULL),
	  fFontFiles(1024)
{
	// initialize engine
	FT_Error error = FT_Init_FreeType(&fLibrary);
	if (error)
		fprintf(stderr, "Could not initialise FreeType library\n");

	// start thread to scan font files
	thread_id fontScanner = spawn_thread(_UpdateThreadEntry, threadName, B_LOW_PRIORITY,
		this);
	if (fontScanner >= B_OK)
		resume_thread(fontScanner);

	Run();
}

// destructor
FontRegistry::~FontRegistry()
{
	_MakeEmpty();
}

// MessageReceived
void
FontRegistry::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_UPDATE: {
			// notify potential observes
			BMessage message(MSG_FONTS_CHANGED);
			SendNotices(FONTS_CHANGED, &message);
			break;
		}
		default:
			break;
	}
}

// CreateDefault
void
FontRegistry::CreateDefault()
{
	if (sDefaultRegistry == NULL)
		sDefaultRegistry = new FontRegistry();
}

// DeleteDefault
void
FontRegistry::DeleteDefault()
{
	sDefaultRegistry->Lock();
	sDefaultRegistry->Quit();
	sDefaultRegistry = NULL;
}

// Default
FontRegistry*
FontRegistry::Default()
{
	CreateDefault();
	return sDefaultRegistry;
}

// FontFileAt
const char*
FontRegistry::FontFileAt(int32 index) const
{
	if (font_file* ff = (font_file*)fFontFiles.ItemAt(index)) {
		return ff->path.String();
	}
	return NULL;
}

// FontFileFor
const char*
FontRegistry::FontFileFor(const char* family, const char* style) const
{
	if (!family || !style)
		return NULL;

	int32 count = fFontFiles.CountItems();
	for (int32 i = 0; i < count; i++) {
		font_file* ff = (font_file*)fFontFiles.ItemAtFast(i);
		if (ff->family_name && strcmp(ff->family_name, family) == 0
			&& ff->style_name && strcmp(ff->style_name, style) == 0)
			return ff->path.String();
	}

//	BString missing(family);
//	missing << '/' << style;
//	if (!fMissingFontFiles.Contains(missing.String())) {
//		fprintf(stderr, "No font file path for %s/%s\n", family, style);
//		fMissingFontFiles.Add(missing.String());
//	}

	return NULL;
}

// IndexFor
int32
FontRegistry::IndexFor(const char* family, const char* style) const
{
	if (!family || !style)
		return -1;
	int32 count = fFontFiles.CountItems();
	for (int32 i = 0; i < count; i++) {
		font_file* ff = (font_file*)fFontFiles.ItemAtFast(i);
		if (ff->family_name && strcmp(ff->family_name, family) == 0
			&& ff->style_name && strcmp(ff->style_name, style) == 0)
			return i;
	}
	return -1;
}

// FamilyFor
const char*
FontRegistry::FamilyFor(const char* fontFile) const
{
	if (font_file* ff = _FontFileFor(fontFile))
		return ff->family_name;
	return NULL;
}

// StyleFor
const char*
FontRegistry::StyleFor(const char* fontFile) const
{
	if (font_file* ff = _FontFileFor(fontFile))
		return ff->style_name;
	return NULL;
}

// FullFamilyFor
const char*
FontRegistry::FullFamilyFor(const char* fontFile) const
{
	if (font_file* ff = _FontFileFor(fontFile))
		return ff->full_family_name;
	return NULL;
}

// PostScriptNameFor
const char*
FontRegistry::PostScriptNameFor(const char* fontFile) const
{
	if (font_file* ff = _FontFileFor(fontFile))
		return ff->ps_name;
	return NULL;
}

// CountFontFiles
int32
FontRegistry::CountFontFiles() const
{
	return fFontFiles.CountItems();
}

// GetFontAt
bool
FontRegistry::GetFontAt(int32 index, char* family, char* style) const
{
	font_file* file = (font_file*)fFontFiles.ItemAt(index);
	if (!file)
		return false;

	if (family) {
		if (!file->family_name)
			return false;
		sprintf(family, "%s", file->family_name);
	}

	if (style) {
		if (!file->style_name)
			return false;
		sprintf(style, "%s", file->style_name);
	}

	return true;
}

//// PopulateMenu
//void
//FontRegistry::PopulateMenu(BMenu* menu, bool subMenus,
//	const char* markedFamily, const char* markedStyle)
//{
//	if (!menu || !markedFamily || !markedStyle || !Lock())
//		return;
//
//	if (subMenus) {
//		BMenu* fontMenu = NULL;
//		int32 count = fFontFiles.CountItems();
//		for (int32 i = 0; i < count; i++) {
//			font_file* ff = (font_file*)fFontFiles.ItemAtFast(i);
//
//			BMessage* message = new BMessage(MSG_SET_FONT);
//			message->AddString("font family", ff->family_name);
//			message->AddString("font style", ff->style_name);
//
//			FontMenuItem* item = new FontMenuItem(ff->style_name,
//												  ff->family_name,
//												  ff->style_name,
//												  message);
//			bool markStyle = false;
//			if (!fontMenu || (fontMenu->Name() && ff->family_name
//				&& strcmp(fontMenu->Name(), ff->family_name) != 0)) {
//				// create new entry
//				fontMenu = new BMenu(ff->family_name);
//				fontMenu->AddItem(item);
//				menu->AddItem(fontMenu);
//				// mark the menu if necessary
//				if (strcmp(markedFamily, ff->family_name) == 0) {
//					if (BMenuItem* superItem = fontMenu->Superitem())
//						superItem->SetMarked(true);
//					markStyle = true;
//				}
//			} else {
//				// reuse old entry
//				fontMenu->AddItem(item);
//			}
//			// mark the item if necessary
//			if (markStyle && strcmp(markedStyle, ff->style_name) == 0)
//				item->SetMarked(true);
//		}
//	} else {
//		const char* previousFamily = NULL;
//		int32 count = fFontFiles.CountItems();
//		for (int32 i = 0; i < count; i++) {
//			font_file* ff = (font_file*)fFontFiles.ItemAtFast(i);
//
//			if (previousFamily
//				&& strcmp(previousFamily, ff->family_name) != 0)
//				menu->AddSeparatorItem();
//			BString label;
//			label << ff->family_name << ", " << ff->style_name;
//
//			BMessage* message = new BMessage(MSG_SET_FONT);
//			message->AddString("font family", ff->family_name);
//			message->AddString("font style", ff->style_name);
//
//			FontMenuItem* item = new FontMenuItem(label.String(),
//												  ff->family_name,
//												  ff->style_name,
//												  message);
//			menu->AddItem(item);
//			if (strcmp(markedFamily, ff->family_name) == 0
//				&& strcmp(markedStyle, ff->style_name) == 0)
//				item->SetMarked(true);
//			previousFamily = ff->family_name;
//		}
//	}
//	Unlock();
//}

// _MakeEmpty
void
FontRegistry::_MakeEmpty()
{
	int32 i = fFontFiles.CountItems() - 1;
	while (i >= 0) {
		if (font_file* ff = (font_file*)fFontFiles.ItemAt(i)) {
			_DeleteFontFile(ff);
		}
		i--;
	}
	fFontFiles.MakeEmpty();
}

// _DeleteFontFile
void
FontRegistry::_DeleteFontFile(font_file* ff) const
{
	free(ff->family_name);
	free(ff->style_name);
	free(ff->full_family_name);
	free(ff->ps_name);
	delete ff;
}

// _UpdateThreadEntry
int32
FontRegistry::_UpdateThreadEntry(void* cookie)
{
	FontRegistry* fm = (FontRegistry*)cookie;
	if (fm && fm->Lock()) {
//bigtime_t now = system_time();
		// update from system, common and user fonts folders
		BPath path;
		if (find_directory(B_SYSTEM_FONTS_DIRECTORY, &path) == B_OK) {
			BDirectory fontFolder(path.Path());
			fm->_Update(&fontFolder);
		}
		if (find_directory(B_COMMON_FONTS_DIRECTORY, &path) == B_OK) {
			BDirectory fontFolder(path.Path());
			fm->_Update(&fontFolder);
		}
		if (find_directory(B_USER_FONTS_DIRECTORY, &path) == B_OK) {
			BDirectory fontFolder(path.Path());
			fm->_Update(&fontFolder);
		}
/*printf("scanning fonts: %lld µsecs\n", system_time() - now);
for (int32 i = 0; font_file* ff = (font_file*)fFontFiles.ItemAt(i); i++) {
	printf("fond %ld: \"%s, %s\"\n", i, ff->family_name, ff->style_name);
}*/
		fm->Unlock();
	}
	return 0;
}

//_Update
void
FontRegistry::_Update(BDirectory* fontFolder)
{
	fontFolder->Rewind();
	// scan the entire folder for font files
	BEntry entry;
	while (fontFolder->GetNextEntry(&entry) >= B_OK) {
		if (entry.IsDirectory()) {
			// recursive scan of sub folders
			BDirectory subFolder(&entry);
//entry_ref ref;
//entry.GetRef(&ref);
//printf("scanning subfolder: %s\n", ref.name);
			_Update(&subFolder);
		} else {
			_AddFont(entry);
		}
	}
}

// _ExtractFontNames
void
FontRegistry::_ExtractFontNames(FT_Face face, font_file* fontFile,
	int32 nameCount) const
{
	FT_SfntName fontName;
	for (int32 i = 0;
		(fontFile->full_family_name == NULL || fontFile->ps_name == NULL)
		&& i < nameCount; i++) {

		if (FT_Get_Sfnt_Name(face, i, &fontName ) == 0) {
			switch (fontName.platform_id) {
				case TT_PLATFORM_APPLE_UNICODE:
//printf("  platform: TT_PLATFORM_APPLE_UNICODE\n");
					// TODO: implement
					break;
				case TT_PLATFORM_MACINTOSH:
//printf("  platform: TT_PLATFORM_MACINTOSH\n");
					if (fontName.encoding_id == TT_MAC_ID_ROMAN) {
						if (fontName.name_id == TT_NAME_ID_FONT_FAMILY) {
							fontFile->full_family_name
								= (char*)malloc(fontName.string_len + 1);
							memcpy(fontFile->full_family_name, fontName.string,
								fontName.string_len);
							fontFile->full_family_name[fontName.string_len] = 0;

							if (fontFile->family_name)
								free(fontFile->family_name);
							fontFile->family_name
								= strdup(fontFile->full_family_name);
						} else if (fontName.name_id == TT_NAME_ID_PS_NAME) {
							fontFile->ps_name
								= (char*)malloc(fontName.string_len + 1);
							memcpy(fontFile->ps_name, fontName.string,
								fontName.string_len);
							fontFile->ps_name[fontName.string_len] = 0;
						}
					}
					break;
				case TT_PLATFORM_MICROSOFT:
//printf("  platform: TT_PLATFORM_MICROSOFT\n");
					// TODO: implement
					if (fontName.encoding_id == TT_MS_ID_UNICODE_CS) {
						if (fontName.name_id == TT_NAME_ID_FONT_FAMILY
							|| fontName.name_id == TT_NAME_ID_PS_NAME) {
							int32 length = fontName.string_len + 1;
							char* buffer = (char*)malloc(length);
							memcpy(buffer, fontName.string, length - 1);
							buffer[length - 1] = 0;
							int32 state = 0;
							convert_to_utf8(B_UNICODE_CONVERSION, buffer,
								&length, buffer, &length, &state, B_SUBSTITUTE);
							if (fontName.name_id == TT_NAME_ID_FONT_FAMILY)
								fontFile->full_family_name = buffer;
							else if (fontName.name_id == TT_NAME_ID_PS_NAME)
								fontFile->ps_name = buffer;
//printf("uni-code name: '%s' (orig-length: %d, length: %ld)\n", buffer,
//fontName.string_len, length);
						}
					}
					break;
				default:
//printf("unkown platform id: %ld\n", fontName.platform_id);
					break;
			}
		}
/*
char* name = (char*)malloc(fontName.string_len + 1);
memcpy(name, fontName.string, fontName.string_len);
name[fontName.string_len] = 0;
printf("  %d, %d, ", fontName.encoding_id, fontName.name_id);
printf("name: '%s'\n", name);
free(name);
*/
	}
}

static int32
next_token(const char* string, BString& token)
{
	if (!string[0])
		return 0;
	token = "";
	token << string[0];
	int32 i = 1;
	while (string[i] > 96) {
		token << string[i];
		i++;
	}
	if (token.Length() > 1) {
		// replace a number of known short verions of style names
		if (token == "Cond")
			token = "Condensed";
		else if (token == "Ext")
			token = "Extra";
		else if (token == "Ital")
			token = "Italic";
		else if (token == "Lig")
			token = "Light";
		else if (token == "Obl")
			token = "Oblique";
	}
	return i;
}

// _ImproveStyleNameFromPostScriptName
void
FontRegistry::_ImproveStyleNameFromPostScriptName(font_file* fontFile) const
{
//printf("A-Z: %d-%d, a-z: %d-%d\n", 'A', 'Z', 'a', 'z');

	BString psName = fontFile->ps_name;
	int32 styleStart = psName.FindFirst('-');
	if (styleStart < 0) {
//printf("ps name does not contain '-': %s\n", psName.String());
		return;
	}
	styleStart++;
	const char* s = psName.String() + styleStart;
	BString token;
	BString styleName;
	int32 skip = next_token(s, token);
	while (skip > 0) {
		if (skip > 1) {
			if (styleName.Length() > 0)
				styleName << " ";
			styleName << token;
		}
		s += skip;
		skip = next_token(s, token);
	}
//printf("%s -> %s\n", psName.String(), styleName.String());
	if (styleName.Length() > 0) {
		free(fontFile->style_name);
		fontFile->style_name = strdup(styleName.String());
	}
}

// _AddFont
void
FontRegistry::_AddFont(const BEntry& entry)
{
	// see if this is a usable font file
	BPath path;
	if (entry.GetPath(&path) < B_OK)
		return;

	FT_Face face;
	FT_Error error = FT_New_Face(fLibrary, path.Path(), 0, &face);
	if (error || !face->family_name || !face->style_name) {
		if (!error)
			FT_Done_Face(face);
		return;
	}

	// create font_file object and init it
	font_file* fontFile = new font_file;
	fontFile->path = path.Path();
	fontFile->family_name = strdup(face->family_name);
	fontFile->style_name = strdup(face->style_name);
	fontFile->full_family_name = NULL;
	fontFile->ps_name = strdup(FT_Get_Postscript_Name(face));
	// iterate over the names we find for this font face
	int32 nameCount = FT_Get_Sfnt_Name_Count(face);
	if (nameCount == 0) {
		_ImproveStyleNameFromPostScriptName(fontFile);
	} else {
		_ExtractFontNames(face, fontFile, nameCount);
	}
	// find slot in sorted list and add it
	int32 index = 0;
	bool duplicate = false;
	font_file* ff;
	while ((ff = (font_file*)fFontFiles.ItemAt(index))
		   && strcasecmp(fontFile->family_name, ff->family_name) >= 0) {
		if (strcasecmp(fontFile->family_name, ff->family_name) == 0) {
			while ((ff = (font_file*)fFontFiles.ItemAt(index))
				   && strcasecmp(fontFile->family_name, ff->family_name) == 0
				   && strcasecmp(fontFile->style_name, ff->style_name) >= 0) {
				if (strcasecmp(fontFile->style_name, ff->style_name) == 0) {
					duplicate = true;
					break;
				} else
				   index++;
			}
			break;
	   }
	   index++;
	}

	if (!duplicate) {
		fFontFiles.AddItem((void*)fontFile, index);
	} else {
		_DeleteFontFile(fontFile);
	}

	FT_Done_Face(face);
}


// _FontFileFor
FontRegistry::font_file*
FontRegistry::_FontFileFor(const char* path) const
{
	if (!path)
		return NULL;

	int32 count = fFontFiles.CountItems();
	for (int32 i = 0; i < count; i++) {
		font_file* ff = (font_file*)fFontFiles.ItemAtFast(i);
		if (ff->path == path)
			return ff;
	}

	return NULL;
}
