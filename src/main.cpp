
#include <stdio.h>
#include <stdlib.h>

#include "App.h"

// main
int
main(int argc, const char* argv[])
{
	App app(BRect(0, 0, 799, 599));
	app.Run();
	return 0;
}
