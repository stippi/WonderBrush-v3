/*
 * Copyright 2001-2005, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Erik Jaesler (erik@cgsoftware.com)
 */

/**	Extra messaging utility functions */

#include <string.h>
#include <ByteOrder.h>

#include <MessageUtils.h>

namespace BPrivate {

uint32
CalculateChecksum(const uint8 *buffer, int32 size)
{
	uint32 sum = 0;
	uint32 temp = 0;

	while (size > 3) {
#if defined(__INTEL__)
		sum += B_SWAP_INT32(*(int32 *)buffer);
#else
		sum += *(int32 *)buffer;
#endif
		buffer += 4;
		size -= 4;
	}

	while (size > 0) {
		temp = (temp << 8) + *buffer++;
		size -= 1;
	}

	return sum + temp;
}

} // namespace BPrivate
