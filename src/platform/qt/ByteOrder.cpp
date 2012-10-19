/*
 * Copyright 2004-2005, Axel Dörfler, axeld@pinc-software.de. All rights reserved.
 * Distributed under the terms of the MIT License.
 */


#include <ByteOrder.h>


status_t
swap_data(type_code type, void *_data, size_t length, swap_action action)
{
	// is there anything to do?
#if B_HOST_IS_LENDIAN
	if (action == B_SWAP_HOST_TO_LENDIAN || action == B_SWAP_LENDIAN_TO_HOST)
		return B_OK;
#else
	if (action == B_SWAP_HOST_TO_BENDIAN || action == B_SWAP_BENDIAN_TO_HOST)
		return B_OK;
#endif

	if (length == 0)
		return B_OK;

	if (_data == NULL)
		return B_BAD_VALUE;

	// ToDo: these are not safe. If the length is smaller than the size of
	// the type to be converted, too much data may be read. R5 behaves in the
	// same way though.
	switch (type) {
		// 16 bit types
		case B_INT16_TYPE:
		case B_UINT16_TYPE:
		{
			uint16 *data = (uint16 *)_data;
			uint16 *end = (uint16 *)((addr_t)_data + length);

			while (data < end) {
				*data = __swap_int16(*data);
				data++;
			}
			break;
		}

		// 32 bit types
		case B_FLOAT_TYPE:
		case B_INT32_TYPE:
		case B_UINT32_TYPE:
		case B_SIZE_T_TYPE:
		case B_SSIZE_T_TYPE:
		case B_TIME_TYPE:
		case B_POINTER_TYPE:
		case B_RECT_TYPE:
		case B_POINT_TYPE:
		{
			// ToDo: some of these types may not be 32-bit on 64-bit platforms!
			uint32 *data = (uint32 *)_data;
			uint32 *end = (uint32 *)((addr_t)_data + length);

			while (data < end) {
				*data = __swap_int32(*data);
				data++;
			}
			break;
		}

		// 64 bit types
		case B_DOUBLE_TYPE:
		case B_INT64_TYPE:
		case B_UINT64_TYPE:
		case B_OFF_T_TYPE:
		{
			uint64 *data = (uint64 *)_data;
			uint64 *end = (uint64 *)((addr_t)_data + length);

			while (data < end) {
				*data = __swap_int64(*data);
				data++;
			}
			break;
		}

		default:
			// not swappable or recognized type!
			return B_BAD_VALUE;
	}

	return B_OK;
}


bool
is_type_swapped(type_code type)
{
	// Returns true when the type is in the host's native format
	// Looks like a pretty strange function to me :)

	switch (type) {
		case B_BOOL_TYPE:
		case B_CHAR_TYPE:
		case B_COLOR_8_BIT_TYPE:
		case B_DOUBLE_TYPE:
		case B_FLOAT_TYPE:
		case B_GRAYSCALE_8_BIT_TYPE:
		case B_INT64_TYPE:
		case B_INT32_TYPE:
		case B_INT16_TYPE:
		case B_INT8_TYPE:
		case B_MESSAGE_TYPE:
		case B_MESSENGER_TYPE:
		case B_MIME_TYPE:
		case B_MONOCHROME_1_BIT_TYPE:
		case B_OFF_T_TYPE:
		case B_PATTERN_TYPE:
		case B_POINTER_TYPE:
		case B_POINT_TYPE:
		case B_RECT_TYPE:
		case B_REF_TYPE:
		case B_RGB_32_BIT_TYPE:
		case B_RGB_COLOR_TYPE:
		case B_SIZE_T_TYPE:
		case B_SSIZE_T_TYPE:
		case B_STRING_TYPE:
		case B_TIME_TYPE:
		case B_UINT64_TYPE:
		case B_UINT32_TYPE:
		case B_UINT16_TYPE:
		case B_UINT8_TYPE:
			return true;
	}

	return false;
}


uint16
__swap_int16(uint16 value)
{
	return (value >> 8) | (value << 8);
}


uint32
__swap_int32(uint32 value)
{
	return (value >> 24) | ((value & 0xff0000) >> 8) | ((value & 0xff00) << 8)
		| (value << 24);
}


uint64
__swap_int64(uint64 value)
{
	return uint64(__swap_int32(uint32(value >> 32)))
		| (uint64(__swap_int32(uint32(value))) << 32);
}
