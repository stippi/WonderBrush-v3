#ifndef PLATFORM_HAIKU_WONDER_BRUSH_H
#define PLATFORM_HAIKU_WONDER_BRUSH_H


#include <stdio.h>
#include <string.h>

#include <Application.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>


enum {
	MSG_NEW_WINDOW	= 'nwnd',
	MSG_WINDOW_QUIT	= 'wndq'
};


template<typename BaseClass>
class PlatformWonderBrush : public BApplication, protected BaseClass {
public:
	using BaseClass::NewWindow;
	using BaseClass::WindowQuit;
	using BaseClass::StoreSettings;
	using BaseClass::RestoreSettings;

	PlatformWonderBrush(int argc, char** argv, BRect bounds)
		:
		BApplication("application/x-vnd.Yellowbites.WonderBrush2"),
		BaseClass(bounds)
	{
	}

	virtual void MessageReceived(BMessage* message)
	{
		switch (message->what) {
			case MSG_NEW_WINDOW:
				NewWindow();
				break;
			case MSG_WINDOW_QUIT:
			{
				WindowQuit(message);
				if (BaseClass::fWindowCount == 0)
					PostMessage(B_QUIT_REQUESTED, this);
				break;
			}
			default:
				BApplication::MessageReceived(message);
				break;
		}
	}

	virtual void ReadyToRun()
	{
		RestoreSettings();
		NewWindow();
	}

	virtual	bool QuitRequested()
	{
		StoreSettings();
		return BApplication::QuitRequested();
	}

protected:
	status_t OpenSettingsFile(BFile& file, bool forWriting)
	{
		BPath path;
		status_t ret = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
		if (ret != B_OK) {
			fprintf(stderr, "Failed to find the user settings directory: %s\n",
				strerror(ret));
			return ret;
		}
		ret = path.Append("WonderBrush3");
		if (ret != B_OK) {
			fprintf(stderr, "Failed to initialize the settings path: %s\n",
				strerror(ret));
			return ret;
		}

		ret = create_directory(path.Path(), 0777);
		if (ret != B_OK) {
			fprintf(stderr, "Failed to create the settings path: %s\n",
				strerror(ret));
			return ret;
		}

		ret = path.Append("main_settings");
		if (ret != B_OK) {
			fprintf(stderr, "Failed to initialize the settings path: %s\n",
				strerror(ret));
			return ret;
		}

		if (forWriting) {
			ret = file.SetTo(path.Path(), B_CREATE_FILE | B_ERASE_FILE
				| B_WRITE_ONLY);
		} else {
			ret = file.SetTo(path.Path(), B_READ_ONLY);
		}

		if (ret != B_OK) {
			if (ret != B_ENTRY_NOT_FOUND) {
				fprintf(stderr, "Failed to initialize the settings file (%s): "
					"%s\n", path.Path(), strerror(ret));
			}
			return ret;
		}

		return B_OK;
	}
};


#endif	// PLATFORM_HAIKU_WONDER_BRUSH_H