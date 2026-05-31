#ifndef WB_TEST_STUB_LOCKER_H
#define WB_TEST_STUB_LOCKER_H

class BLocker {
public:
	BLocker() {}
	bool Lock() { return true; }
	void Unlock() {}
};

#endif // WB_TEST_STUB_LOCKER_H
