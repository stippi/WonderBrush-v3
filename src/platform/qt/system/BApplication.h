#ifndef PLATFORM_QT_B_APPLICATION_H
#define PLATFORM_QT_B_APPLICATION_H


#include <Looper.h>

#include <QApplication>


static inline BLooper*
_PlatformBeApp()
{
	return dynamic_cast<BLooper*>(QApplication::instance());
}


static inline BMessenger
_PlatformBeAppMessenger()
{
	return BMessenger(_PlatformBeApp());
}

#define be_app				_PlatformBeApp()
#define be_app_messenger	_PlatformBeAppMessenger()


#endif // PLATFORM_QT_B_APPLICATION_H
