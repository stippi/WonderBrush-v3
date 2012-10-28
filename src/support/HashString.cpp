/*
 * Copyright 2002-2007, Ingo Weinhold <ingo_weinhold@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include <stdlib.h>
#include <string.h>

#include "HashString.h"


/*!
	\class HashString
	\brief A very simple string class.
*/


// sEmptyString
HashString::Data HashString::sEmptyString = {
	1,			// refCount
	{ '\0' }	// data[1]
};


// SetTo
bool
HashString::SetTo(const char* string, int32 maxLength)
{
	// compute the actual string length
	if (string) {
		if (maxLength > 0)
			maxLength = strnlen(string, maxLength);
		else if (maxLength < 0)
			maxLength = strlen(string);
	}

	// get/allocate the data
	Data* data;
	if (maxLength > 0) {
		data = (Data*)malloc(sizeof(Data) + maxLength);
		if (data == NULL)
			return false;

		data->refCount = 1;
		memcpy(data->data, string, maxLength);
		data->data[maxLength] = '\0';
	} else {
		data = &sEmptyString;
		atomic_add(&data->refCount, 1);
	}

	// set the new data
	_Unset();

	fData = data;
	fLength = maxLength;

	return true;
}


// Truncate
bool
HashString::Truncate(int32 newLength)
{
	if (newLength <= 0) {
		newLength = 0;
		SetTo(NULL, 0);
		return true;
	}

	if (newLength < fLength) {
		if (atomic_add(&fData->refCount, 0) > 1)
			return SetTo(GetString(), newLength);

		// we have the last reference
		fLength = newLength;
		fData->data[fLength] = '\0';
	}

	return true;
}


// ==
bool
HashString::operator==(const HashString& other) const
{
	return (fLength == other.fLength
		&& (strcmp(fData->data, other.fData->data) == 0));
}


// ==
bool
HashString::operator==(const char* other) const
{
	return (strncmp(fData->data, other, fLength) == 0
		&& other[fLength] == '\0');
}


// _Unset
void
HashString::_Unset()
{
	if (fData != NULL) {
		if (atomic_add(&fData->refCount, -1) == 1)
			free(fData);
	}
}

