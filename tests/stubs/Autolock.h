#ifndef WB_TEST_STUB_AUTOLOCK_H
#define WB_TEST_STUB_AUTOLOCK_H

#include <Locker.h>

class BAutolock {
public:
	BAutolock(BLocker* locker)
		: fLocker(locker)
	{
		if (fLocker != 0)
			fLocker->Lock();
	}

	BAutolock(BLocker& locker)
		: fLocker(&locker)
	{
		fLocker->Lock();
	}

	~BAutolock()
	{
		if (fLocker != 0)
			fLocker->Unlock();
	}

private:
	BLocker* fLocker;
};

#endif // WB_TEST_STUB_AUTOLOCK_H
