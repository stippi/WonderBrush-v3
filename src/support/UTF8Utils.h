/*
 * UTF8ToCharCode.h
 *
 * Copyright 2004-2010, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 */

#ifndef UTF8_TO_CHAR_CODE_H_
#define UTF8_TO_CHAR_CODE_H_


/*!	UTF8ToCharCode converts the input that includes potential multibyte chars
	to UTF-32 char codes that can be used by FreeType. The string pointer is
	then advanced to the next character in the string. In case the terminating
	0 is reached, the string pointer is not advanced anymore and nulls are
	returned. This makes it safe to overruns and enables streamed processing
	of UTF8 strings.
*/
inline unsigned
UTF8ToCharCode(const char** bytes)
{
	#define UTF8_SUBSTITUTE_CHARACTER	0xfffd

	unsigned result;
	if (((*bytes)[0] & 0x80) == 0) {
		// a single byte character
		result = (*bytes)[0];
		if (result != '\0') {
			// do not advance beyond the terminating '\0'
			(*bytes)++;
		}

		return result;
	}

	if (((*bytes)[0] & 0xc0) == 0x80) {
		// not a proper multibyte start
		(*bytes)++;
		return UTF8_SUBSTITUTE_CHARACTER;
	}

	// start of a multibyte character
	unsigned char mask = 0x80;
	result = (unsigned)((*bytes)[0] & 0xff);
	(*bytes)++;

	while (result & mask) {
		if (mask == 0x02) {
			// seven byte char - invalid
			return UTF8_SUBSTITUTE_CHARACTER;
		}

		result &= ~mask;
		mask >>= 1;
	}

	while (((*bytes)[0] & 0xc0) == 0x80) {
		result <<= 6;
		result += (*bytes)[0] & 0x3f;
		(*bytes)++;

		mask <<= 1;
		if (mask == 0x40)
			return result;
	}

	if (mask == 0x40)
		return result;

	if ((*bytes)[0] == '\0') {
		// string terminated within multibyte char
		return 0x00;
	}

	// not enough bytes in multibyte char
	return UTF8_SUBSTITUTE_CHARACTER;

	#undef UTF8_SUBSTITUTE_CHARACTER
}


#endif /* UTF8TOCHARCODE_H_ */
