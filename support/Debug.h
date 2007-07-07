// Debug.h

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

inline void nodebug(...)
{
}

#define debug printf
//#define debug nodebug


#endif	// DEBUG_H
