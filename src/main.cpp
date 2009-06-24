
#include <stdio.h>
#include <stdlib.h>

#include "WonderBrush.h"

// main
int
main(int argc, const char* argv[])
{
	WonderBrush app(BRect(0, 0, 799, 599));
	app.Run();
	return 0;
}
