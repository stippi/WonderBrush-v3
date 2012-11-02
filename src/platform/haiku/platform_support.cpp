#include "support.h"

#include <stdlib.h>

#include <Application.h>
#include <Resources.h>
#include <Roster.h>
#include <UTF8.h>


int32
get_optimal_worker_thread_count()
{
	system_info info;
	get_system_info(&info);
	return info.cpu_count;
}


status_t
get_app_resources(BResources& resources)
{
	app_info info;
	status_t status = be_app->GetAppInfo(&info);
	if (status != B_OK)
		return status;

	return resources.SetTo(&info.ref);
}


char*
convert_utf16_to_utf8(const void* string, size_t length)
{
	// Worst case is UTF-8 needs 3 bytes for an UTF-16 char.
	int32 destLength = length * 3 / 2;
	char* buffer = (char*)malloc(destLength + 1);
	if (buffer == NULL)
		return NULL;

	// convert
	int32 sourceLength = length;
	int32 state = 0;
	convert_to_utf8(B_UNICODE_CONVERSION, (const char*)string,
		&sourceLength, buffer, &destLength, &state, B_SUBSTITUTE);

	// null-terminate
	buffer[destLength] = '\0';

	return buffer;
}
