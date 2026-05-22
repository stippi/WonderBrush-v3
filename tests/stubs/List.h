#ifndef WB_TEST_STUB_LIST_H
#define WB_TEST_STUB_LIST_H

#include <SupportDefs.h>

class BList {
public:
	BList(int32 = 20) {}
	BList(const BList&) {}
	~BList() {}
	bool AddItem(void*) { return true; }
	bool AddItem(void*, int32) { return true; }
	bool RemoveItem(void*) { return true; }
	void* RemoveItem(int32) { return 0; }
	bool HasItem(void*) const { return false; }
	bool HasItem(const void*) const { return false; }
	int32 IndexOf(void*) const { return -1; }
	int32 IndexOf(const void*) const { return -1; }
	void* ItemAt(int32) const { return 0; }
	void* ItemAtFast(int32) const { return 0; }
	int32 CountItems() const { return 0; }
};

#endif // WB_TEST_STUB_LIST_H
