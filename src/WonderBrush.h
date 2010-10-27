#ifndef WONDER_BRUSH_H
#define WONDER_BRUSH_H

#include <Application.h>
#include <Rect.h>

class BFile;
class Document;
class Layer;

enum {
	MSG_NEW_WINDOW	= 'nwnd',
	MSG_WINDOW_QUIT	= 'wndq'
};

class WonderBrush : public BApplication {
public:
								WonderBrush(BRect bounds);

	virtual	void				MessageReceived(BMessage* message);
	virtual void				ReadyToRun();
	virtual	bool				QuitRequested();

private:
			void				_NewWindow();

			status_t			_OpenSettingsFile(BFile& file,
									bool forWriting);
			void				_StoreSettings();
			void				_RestoreSettings();

private:
			BMessage			fSettings;

			Document*			fDocument;
			Layer*				fEditLayer;

			BRect				fWindowFrame;
			int32				fWindowCount;
};

#endif // WONDER_BRUSH_H
