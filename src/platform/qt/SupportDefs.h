/*
 * Copyright 2004-2010, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *		Erik Jaesler (erik@cgsoftware.com)
 */
#ifndef _SUPPORT_DEFS_H
#define _SUPPORT_DEFS_H


//#include <BeBuild.h>
#include <Errors.h>

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <sys/types.h>


/* fixed-size integer types */
typedef	int8_t			int8;
typedef uint8_t			uint8;
typedef	int16_t			int16;
typedef uint16_t		uint16;
typedef	int32_t			int32;
typedef uint32_t		uint32;
typedef	int64_t			int64;
typedef uint64_t		uint64;

/* shorthand types */
typedef volatile int8   		vint8;
typedef volatile uint8			vuint8;
typedef volatile int16			vint16;
typedef volatile uint16			vuint16;
typedef volatile int32			vint32;
typedef volatile uint32			vuint32;
typedef volatile int64			vint64;
typedef volatile uint64			vuint64;

typedef volatile long			vlong;
typedef volatile int			vint;
typedef volatile short			vshort;
typedef volatile char			vchar;

typedef volatile unsigned long	vulong;
typedef volatile unsigned int	vuint;
typedef volatile unsigned short	vushort;
typedef volatile unsigned char	vuchar;

typedef unsigned char			uchar;
typedef unsigned short          unichar;

/* descriptive types */
typedef int32					status_t;
typedef int64					bigtime_t;
typedef int64					nanotime_t;
typedef uint32					type_code;
typedef uint32					perform_code;
typedef size_t					addr_t;

//typedef __haiku_phys_addr_t		phys_addr_t;
//typedef phys_addr_t				phys_size_t;

//typedef	__haiku_generic_addr_t	generic_addr_t;
//typedef	generic_addr_t			generic_size_t;


/* printf()/scanf() format strings for [u]int* types */
#define B_PRId8			"d"
#define B_PRIi8			"i"
#define B_PRId16		"d"
#define B_PRIi16		"i"
#define B_PRId32		PRId32
#define B_PRIi32		PRIi32
#define B_PRId64		PRId64
#define B_PRIi64		PRIi64
#define B_PRIu8			"u"
#define B_PRIo8			"o"
#define B_PRIx8			"x"
#define B_PRIX8			"X"
#define B_PRIu16		"u"
#define B_PRIo16		"o"
#define B_PRIx16		"x"
#define B_PRIX16		"X"
#define B_PRIu32		PRIu32
#define B_PRIo32		PRIo32
#define B_PRIx32		PRIx32
#define B_PRIX32		PRIX32
#define B_PRIu64		PRIu64
#define B_PRIo64		PRIo64
#define B_PRIx64		PRIx64
#define B_PRIX64		PRIX64

#define B_SCNd8 		"hhd"
#define B_SCNi8 		"hhi"
#define B_SCNd16		"hd"
#define B_SCNi16	 	"hi"
#define B_SCNd32 		PRId32
#define B_SCNi32	 	PRIi32
#define B_SCNd64		PRId64
#define B_SCNi64 		PRIi64
#define B_SCNu8 		"hhu"
#define B_SCNo8 		"hho"
#define B_SCNx8 		"hhx"
#define B_SCNu16		"hu"
#define B_SCNo16		"ho"
#define B_SCNx16		"hx"
#define B_SCNu32 		PRIu32
#define B_SCNo32 		PRIo32
#define B_SCNx32 		PRIx32
#define B_SCNu64		PRIu64
#define B_SCNo64		PRIo64
#define B_SCNx64		PRIx64

/* printf() format strings for some standard types */
/* size_t */
#define B_PRIuSIZE		PRIuPTR
#define B_PRIoSIZE		PRIoPTR
#define B_PRIxSIZE		PRIxPTR
#define B_PRIXSIZE		PRIXPTR
/* ssize_t */
#define B_PRIdSSIZE		SCNdPTR
#define B_PRIiSSIZE		SCNiPTR
/* addr_t */
#define B_PRIuADDR		B_PRIuSIZE
#define B_PRIoADDR		B_PRIoSIZE
#define B_PRIxADDR		B_PRIxSIZE
#define B_PRIXADDR		B_PRIXSIZE
/* phys_addr_t */
//#define B_PRIuPHYSADDR	__HAIKU_PRI_PREFIX_PHYS_ADDR "u"
//#define B_PRIoPHYSADDR	__HAIKU_PRI_PREFIX_PHYS_ADDR "o"
//#define B_PRIxPHYSADDR	__HAIKU_PRI_PREFIX_PHYS_ADDR "x"
//#define B_PRIXPHYSADDR	__HAIKU_PRI_PREFIX_PHYS_ADDR "X"
/* generic_addr_t */
//#define B_PRIuGENADDR	__HAIKU_PRI_PREFIX_GENERIC_ADDR "u"
//#define B_PRIoGENADDR	__HAIKU_PRI_PREFIX_GENERIC_ADDR "o"
//#define B_PRIxGENADDR	__HAIKU_PRI_PREFIX_GENERIC_ADDR "x"
//#define B_PRIXGENADDR	__HAIKU_PRI_PREFIX_GENERIC_ADDR "X"
/* off_t */
//#define B_PRIdOFF		B_PRId64
//#define B_PRIiOFF		B_PRIi64
/* dev_t */
//#define B_PRIdDEV		B_PRId32
//#define B_PRIiDEV		B_PRIi32
/* ino_t */
//#define B_PRIdINO		B_PRId64
//#define B_PRIiINO		B_PRIi64
/* time_t */
//#define B_PRIdTIME		B_PRId32
//#define B_PRIiTIME		B_PRIi32


/* Empty string ("") */
#ifdef __cplusplus
extern const char *B_EMPTY_STRING;
#endif


/* min and max comparisons */
#ifndef __cplusplus
#	ifndef min
#		define min(a,b) ((a)>(b)?(b):(a))
#	endif
#	ifndef max
#		define max(a,b) ((a)>(b)?(a):(b))
#	endif
#endif

/* min() and max() are functions in C++ */
#define min_c(a,b) ((a)>(b)?(b):(a))
#define max_c(a,b) ((a)>(b)?(a):(b))


/* Grandfathering */
#ifndef __cplusplus
#	include <stdbool.h>
#endif

#ifndef NULL
#	define NULL (0)
#endif


#ifdef __cplusplus
extern "C" {
#endif

/* Atomic functions; previous value is returned */
//extern int32	atomic_set(vint32 *value, int32 newValue);
//extern int32	atomic_test_and_set(vint32 *value, int32 newValue, int32 testAgainst);
//extern int32	atomic_add(vint32 *value, int32 addValue);
//extern int32	atomic_and(vint32 *value, int32 andValue);
//extern int32	atomic_or(vint32 *value, int32 orValue);
//extern int32	atomic_get(vint32 *value);

//extern int64	atomic_set64(vint64 *value, int64 newValue);
//extern int64	atomic_test_and_set64(vint64 *value, int64 newValue, int64 testAgainst);
//extern int64	atomic_add64(vint64 *value, int64 addValue);
//extern int64	atomic_and64(vint64 *value, int64 andValue);
//extern int64	atomic_or64(vint64 *value, int64 orValue);
//extern int64	atomic_get64(vint64 *value);

#ifdef __cplusplus
}
#endif


static inline int32
atomic_test_and_set(vint32 *value, int32 newValue, int32 testAgainst)
{
	return __sync_val_compare_and_swap(value, testAgainst, newValue);
}


static inline int32
atomic_add(vint32 *value, int32 addValue)
{
	return __sync_fetch_and_add(value, addValue);
}


static inline int32
atomic_and(vint32 *value, int32 andValue)
{
	return __sync_fetch_and_and(value, andValue);
}


static inline int32
atomic_or(vint32 *value, int32 orValue)
{
	return __sync_fetch_and_or(value, orValue);
}


static inline int32
atomic_get(vint32 *value)
{
	// No equivalent to atomic_get(). We simulate it via atomic or. On most
	// (all?) 32+ bit architectures aligned 32 bit reads will be atomic anyway,
	// though.
	return __sync_fetch_and_or(value, 0);
}


static inline int32
atomic_set(vint32 *value, int32 newValue)
{
	// No equivalent for atomic_set(). We simulate it via a loop.
	int32 oldValue = atomic_get(value);
	int32 foundValue;
	for (;;) {
		foundValue = atomic_test_and_set(value, newValue, oldValue);
		if (foundValue == oldValue)
			return oldValue;
		oldValue = foundValue;
	}
}


/* Other stuff */
extern void*	get_stack_frame(void);


/* Obsolete or discouraged API */

/* use 'true' and 'false' */
#ifndef FALSE
#	define FALSE	0
#endif
#ifndef TRUE
#	define TRUE		1
#endif


#endif	/* _SUPPORT_DEFS_H */
