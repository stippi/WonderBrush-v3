#include "support.h"

#include <OS.h>


int32
get_optimal_worker_thread_count()
{
	system_info info;
	get_system_info(&info);
	return info.cpu_count;
}
