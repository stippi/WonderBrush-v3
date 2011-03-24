#ifndef CROPPER_H
#define CROPPER_H

#include <Application.h>
#include <Path.h>
#include <String.h>

class Cropper : public BApplication {
public:
								Cropper();

	virtual	void				MessageReceived(BMessage* message);
	virtual void				ReadyToRun();
	virtual	void				ArgvReceived(int32 argc, char** argv);
	virtual	void				RefsReceived(BMessage* message);

private:
			void				_PrintUsage(const char* appPath);
			status_t			_CropImage(const BBitmap* bitmap,
									int width, int height,
									BPath path, const char* originalPath) const;

			int32				fTargetWidth;
			int32				fTargetHeight;
			BPath				fTargetFolder;
			uint32				fTargetFormat;
};

#endif // CROPPER_H
