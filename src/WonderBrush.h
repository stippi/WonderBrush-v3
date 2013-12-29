#ifndef WONDER_BRUSH_H
#define WONDER_BRUSH_H

#include <Entry.h>
#include <Message.h>
#include <Rect.h>

#include "Document.h"

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
class Layer;


class WonderBrushBase {
public:
								WonderBrushBase(BRect bounds);
	virtual						~WonderBrushBase();

protected:
	virtual	void				InitialWindow() = 0;
	virtual	Window*				NewWindow(Document* document) = 0;
	virtual	DocumentRef			NewDocument() = 0;
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
	virtual	void				RefsReceived(BMessage* message);

protected:
	virtual	void				InitialWindow();
	virtual	Window*				NewWindow(Document* document);
	virtual	DocumentRef			NewDocument();
			status_t			OpenDocument(Document* document,
									const entry_ref& ref, bool append);

			status_t			ImportDocument(Document* document,
									const entry_ref& ref) const;

	virtual	void				WindowQuit(BMessage* message);

	virtual	void				StoreSettings();
	virtual	void				RestoreSettings();

	virtual void				NotifyFontsLoaded();

private:
			void				_NotifyFontsLoaded(Layer* layer);
};

#endif // WONDER_BRUSH_H
