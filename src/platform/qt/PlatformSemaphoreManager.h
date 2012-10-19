#ifndef PLATFORM_SEMAPHORE_MANAGER_H
#define PLATFORM_SEMAPHORE_MANAGER_H


#include <OS.h>

#include <QMap>
#include <QMutex>


class PlatformSemaphoreManager {
public:
	static	PlatformSemaphoreManager* Manager();

			sem_id				CreateSemaphore(int32 count, const char* name);
			status_t			DeleteSemaphore(sem_id id);
			status_t			AcquireSemaphore(sem_id id)
									{ return AcquireSemaphore(id, 1, 0, 0); }
			status_t			AcquireSemaphore(sem_id id, int32 count,
									uint32 flags, bigtime_t timeout);
			status_t			ReleaseSemaphore(sem_id id)
									{ return ReleaseSemaphore(id, 1, 0); }
			status_t			ReleaseSemaphore(sem_id id, int32 count,
									uint32 flags);

private:
								PlatformSemaphoreManager();

private:
			class Semaphore;

private:
			QMutex				fMutex;
			QMap<sem_id, Semaphore*> fSemaphores;
			sem_id				fNextSemaphoreId;
};


#endif // PLATFORM_SEMAPHORE_MANAGER_H
