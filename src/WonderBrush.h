#ifndef WONDER_BRUSH_H
#define WONDER_BRUSH_H

#include <Rect.h>

#include "PlatformWonderBrush.h"


class BFile;
class Document;
class Layer;


class WonderBrushBase {
public:
								WonderBrushBase(BRect bounds);

protected:
	virtual	void				NewWindow() = 0;
	virtual	void				WindowQuit(BMessage* message) = 0;

	virtual	status_t			OpenSettingsFile(BFile& file,
									bool forWriting) = 0;
	virtual	void				StoreSettings() = 0;
	virtual	void				RestoreSettings() = 0;

protected:
			BMessage			fSettings;

			Document*			fDocument;
			Layer*				fEditLayer;

			BRect				fWindowFrame;
			int32				fWindowCount;
};


class WonderBrush : public PlatformWonderBrush<WonderBrushBase> {
public:
								WonderBrush(int argc, char** argv,
									BRect bounds);

protected:
	virtual	void				NewWindow();
	virtual	void				WindowQuit(BMessage* message);

	virtual	void				StoreSettings();
	virtual	void				RestoreSettings();
};

#endif // WONDER_BRUSH_H
