#include "support.h"

#include <SupportDefs.h>

#include <QThread>


int32
get_optimal_worker_thread_count()
{
	return QThread::idealThreadCount();
}
