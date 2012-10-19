#include "platform_support.h"

#include <DataIO.h>
#include <String.h>


// write_string
status_t
write_string(BPositionIO* stream, BString& string)
{
	if (!stream)
		return B_BAD_VALUE;

	ssize_t written = stream->Write(string.String(), string.Length());
	if (written > B_OK && written < string.Length())
		written = B_ERROR;
	string.SetTo("");
	return written;
}
