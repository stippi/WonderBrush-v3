#ifndef PLATFORM_HAIKU_WONDER_BRUSH_H
#define PLATFORM_HAIKU_WONDER_BRUSH_H


#include <stdio.h>
#include <string.h>

#include <File.h>

#include <QApplication>
#include <QDir>

#include "FontRegistry.h"
#include "Window.h"


template<typename BaseClass>
class PlatformWonderBrush : public QApplication, protected BaseClass,
	public BLooper {
public:
	using BaseClass::NewWindow;
	using BaseClass::WindowQuit;
	using BaseClass::StoreSettings;
	using BaseClass::RestoreSettings;

	PlatformWonderBrush(int& argc, char** argv, BRect bounds)
		:
		QApplication(argc, argv),
		BaseClass(bounds),
		BLooper("application")
	{
	}

	thread_id Run()
	{
		FontRegistry* registry = FontRegistry::Default();
		if (registry->Lock()) {
			registry->StartWatchingAll(this);
			registry->Scan();
			registry->Unlock();
		}

		RestoreSettings();
		NewWindow();

		exec();
		return -1;
	}

//	virtual	bool QuitRequested()
//	{
//		StoreSettings();
//		return BApplication::QuitRequested();
//	}

protected:
	status_t OpenSettingsFile(BFile& file, bool forWriting)
	{
		// create WonderBrush settings dir, if not existing yet
		QString settingsDirPath = QDir::homePath();
		settingsDirPath += QString::fromUtf8("/.WonderBrush");
		if (!QDir().mkpath(settingsDirPath))
			return B_ERROR;

		// open the file
		QString path = settingsDirPath + QString::fromUtf8("/main_settings");
		status_t error = forWriting
			? file.SetTo(path, B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY)
			: file.SetTo(path, B_READ_ONLY);

		if (error != B_OK) {
			if (error != B_ENTRY_NOT_FOUND) {
				fprintf(stderr, "Failed to initialize the settings file (%s): "
					"%s\n", path.toUtf8().data(),
					strerror(_WONDERBRUSH_TO_NATIVE_ERROR(error)));
			}
			return error;
		}

		return B_OK;
	}
};


#endif	// PLATFORM_HAIKU_WONDER_BRUSH_H
