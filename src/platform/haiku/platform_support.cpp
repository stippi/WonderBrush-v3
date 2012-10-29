#include "support.h"

#include <Application.h>
#include <Resources.h>
#include <Roster.h>


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
