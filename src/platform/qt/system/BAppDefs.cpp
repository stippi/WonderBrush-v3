#include "BAppDefs.h"

#include <Cursor.h>

#include <stddef.h>


const BCursor*
_PlatformSystemDefaultCursor()
{
	static BCursor cursor(B_CURSOR_ID_SYSTEM_DEFAULT);
	return &cursor;
}
