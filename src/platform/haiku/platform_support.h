#ifndef PLATFORM_SUPPORT_H
#define PLATFORM_SUPPORT_H


#include <SupportDefs.h>


class BPositionIO;
class BString;


status_t write_string(BPositionIO* stream, BString& string);


#endif	// PLATFORM_SUPPORT_H
