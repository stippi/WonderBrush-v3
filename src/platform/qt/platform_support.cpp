#include "support.h"

#include <Resources.h>
#include <SupportDefs.h>

#include <QThread>

#include "PlatformResourceParser.h"


static BResources*
parse_app_resources()
{
	BResources* resources = new(std::nothrow) BResources;
	if (resources == NULL)
		return NULL;

	if (PlatformResourceParser().ParseAppResources(*resources) != B_OK) {
		delete resources;
		return NULL;
	}

	return resources;
}


int32
get_optimal_worker_thread_count()
{
	return QThread::idealThreadCount();
}


status_t
get_app_resources(BResources& resources)
{
	static BResources* appResources = parse_app_resources();

	if (appResources != NULL) {
		resources = *appResources;
		return B_OK;
	}

	return B_ENTRY_NOT_FOUND;
}


char*
convert_utf16_to_utf8(const void* string, size_t length)
{
	// copy into a QString for easy conversion
	QString qString;
	if (((addr_t)string & 0x1) == 0) {
		qString.setUtf16((const ushort*)string, length / 2);
	} else {
		// The source string is not aligned, so we have to copy it.
		QByteArray stringData((const char*)string, length);
		qString.setUtf16((const ushort*)stringData.data(), length / 2);
	}

	// convert to UTF-8 and allocate the result buffer
	QByteArray utf8String = qString.toUtf8();
	char* result = (char*)malloc(utf8String.length() + 1);
	if (result == NULL)
		return NULL;

	memcpy(result, utf8String.data(), utf8String.length() + 1);
	return result;
}
