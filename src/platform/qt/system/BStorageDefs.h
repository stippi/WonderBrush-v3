/*
 * Copyright 2002-2006, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef PLATFORM_QT_STORAGE_DEFS
#define PLATFORM_QT_STORAGE_DEFS


#include <fcntl.h>


// Open Modes
#define B_READ_ONLY 		O_RDONLY	// read only
#define B_WRITE_ONLY 		O_WRONLY 	// write only
#define B_READ_WRITE		O_RDWR   	// read and write

#define	B_FAIL_IF_EXISTS	O_EXCL		// exclusive create
#define B_CREATE_FILE		O_CREAT		// create the file
#define B_ERASE_FILE		O_TRUNC		// erase the file's data
#define B_OPEN_AT_END	   	O_APPEND	// point to the end of the data



#endif	// PLATFORM_QT_STORAGE_DEFS
