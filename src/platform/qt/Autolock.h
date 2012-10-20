#ifndef PLATFORM_QT_AUTOLOCK_H
#define PLATFORM_QT_AUTOLOCK_H


#include <Locker.h>

#include "AutoLocker.h"


// TODO: Not quite sufficient, since BAutolock can also lock BLooper.
typedef AutoLocker<BLocker> BAutolock;


#endif // PLATFORM_QT_AUTOLOCK_H
