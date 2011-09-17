#ifndef RESIZER_H
#define RESIZER_H

#include <Application.h>
#include <Path.h>
#include <String.h>

class RenderBuffer;
class RenderEngine;

class Resizer : public BApplication {
public:
								Resizer();

	virtual	void				MessageReceived(BMessage* message);
	virtual void				ReadyToRun();
	virtual	void				ArgvReceived(int32 argc, char** argv);
	virtual	void				RefsReceived(BMessage* message);

private:
			void				_PrintUsage(const char* appPath);
			status_t			_ResizeImage(const RenderBuffer& buffer,
									RenderEngine& engine, double scale,
									BPath path, const char* originalPath) const;

			bool				fPrintUsage;
			int32				fTargetSize;
			int32				fTargetThumbSize;
			double				fTargetScale;
			double				fTargetThumbScale;
			BString				fTargetTranslator;
			BPath				fTargetFolder;
			BPath				fTargetThumbFolder;
};

#endif // RESIZER_H
