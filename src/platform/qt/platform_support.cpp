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
