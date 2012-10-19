#ifndef ERRORS_H
#define ERRORS_H


#include <errno.h>


#define B_OK	0

// TODO: Find better mappings for B_ERROR, B_NO_INIT, B_NAME_NOT_FOUND,
// B_BAD_SEM_ID.
#if ENOMEM > 0
#	define B_ERROR			-EINVAL
#	define B_NO_INIT		-EINVAL
#	define B_NAME_NOT_FOUND	-EINVAL
#	define B_BAD_SEM_ID		-EINVAL
#	define B_NO_MEMORY		-ENOMEM
#	define B_BAD_VALUE		-EINVAL
#	define B_TIMED_OUT		-ETIMEDOUT
#	define B_WOULD_BLOCK	-EWOULDBLOCK
#else
#	define B_ERROR			EINVAL
#	define B_NO_INIT		EINVAL
#	define B_NAME_NOT_FOUND	EINVAL
#	define B_BAD_SEM_ID		EINVAL
#	define B_NO_MEMORY		ENOMEM
#	define B_BAD_VALUE		EINVAL
#	define B_TIMED_OUT		ETIMEDOUT
#	define B_WOULD_BLOCK	EWOULDBLOCK
#endif


#endif // ERRORS_H
