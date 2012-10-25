/*
 * Copyright 2006-2008, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ICON_UTILS_H
#define _ICON_UTILS_H


#include <SupportDefs.h>


class BBitmap;


class BIconUtils {
public:
	static	status_t			GetVectorIcon(const uint8* buffer,
									size_t size, BBitmap* result);
};


#endif	// _ICON_UTILS_H
