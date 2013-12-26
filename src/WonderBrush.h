#ifndef WONDER_BRUSH_H
#define WONDER_BRUSH_H

#include <Message.h>
#include <Rect.h>

enum {
	MSG_OPEN						= 'open',
	MSG_SAVE						= 'save',
	MSG_SAVE_AS						= 'svas',
	MSG_EXPORT						= 'expt',
	MSG_EXPORT_AS					= 'exas',
	MSG_NEW_WINDOW					= 'nwnd',
	MSG_NEW_DOCUMENT				= 'ndoc',
	MSG_WINDOW_QUIT					= 'wndq'
};


#include "PlatformWonderBrush.h"


enum {
	EXPORT_MODE_MESSAGE = 0,
	EXPORT_MODE_SVG,
	EXPORT_MODE_BITMAP,
	EXPORT_MODE_BITMAP_SET,
	EXPORT_MODE_ICON_ATTR,
	EXPORT_MODE_ICON_MIME_ATTR,
	EXPORT_MODE_ICON_RDEF,
	EXPORT_MODE_ICON_SOURCE,
};


class BFile;
class Document;
class Layer;


class WonderBrushBase {
public:
								WonderBrushBase(BRect bounds);
	virtual						~WonderBrushBase();

protected:
	virtual	void				NewWindow() = 0;
	virtual	void				NewDocument() = 0;
	virtual	void				WindowQuit(BMessage* message) = 0;

	virtual	void				Open(BMessage* message) = 0;
	virtual	void				SaveAs(BMessage* message) = 0;

	virtual	status_t			OpenSettingsFile(BFile& file,
									bool forWriting) = 0;
	virtual	void				StoreSettings() = 0;
	virtual	void				RestoreSettings() = 0;

	virtual void				NotifyFontsLoaded() = 0;

protected:
			BMessage			fSettings;

			Document*			fDocument;
			Layer*				fEditLayer;

			BRect				fWindowFrame;
			int32				fWindowCount;
};


class WonderBrush : public PlatformWonderBrush<WonderBrushBase> {
private:
			typedef PlatformWonderBrush<WonderBrushBase> BaseClass;

public:
								WonderBrush(int& argc, char** argv,
									BRect bounds);

	virtual	void				MessageReceived(BMessage* message);

protected:
	virtual	void				NewWindow();
	virtual	void				NewDocument();
	virtual	void				WindowQuit(BMessage* message);

	virtual	void				StoreSettings();
	virtual	void				RestoreSettings();

	virtual void				NotifyFontsLoaded();

private:
			void				_NotifyFontsLoaded(Layer* layer);
};

#endif // WONDER_BRUSH_H
