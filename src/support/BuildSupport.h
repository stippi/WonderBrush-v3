#ifndef BUILD_DEFS_H
#define BUILD_DEFS_H


// TODO: Those should better be defined by the build system, so we can build
// for Qt on Haiku as well.
#ifdef __HAIKU__
#	define	WONDERBRUSH_PLATFORM_HAIKU	1
#else
#	define	WONDERBRUSH_PLATFORM_QT	1
#endif


#define WONDERBRUSH_MAKE_STRING(string)	#string

#ifdef WONDERBRUSH_PLATFORM_HAIKU
#	define	WONDERBRUSH_PLATFORM_HEADER(header) \
	"header##_haiku.h"
#elif defined(WONDERBRUSH_PLATFORM_QT)
#	define	WONDERBRUSH_PLATFORM_HEADER(header) \
	WONDERBRUSH_MAKE_STRING(header##_qt.h)
#else
#	error Unsupported platform!
#endif


#endif // BUILD_DEFS_H
