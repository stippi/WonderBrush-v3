#ifndef WB_TEST_STUB_MESSAGE_H
#define WB_TEST_STUB_MESSAGE_H

#include <SupportDefs.h>

class BMessage {
public:
	status_t FindString(const char*, const char**) const { return B_ERROR; }
	status_t AddString(const char*, const char*) { return B_OK; }
	status_t AddPointer(const char*, const void*) { return B_OK; }
	status_t AddInt32(const char*, int32) { return B_OK; }
};

#endif // WB_TEST_STUB_MESSAGE_H
