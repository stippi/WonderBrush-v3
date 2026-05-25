#include "Test.h"

#include "SVGException.h"

#include <string.h>
#include <string>

static void
TestSVGExceptionFormattingIsBounded()
{
	std::string longText(8192, 'x');
	agg::svg::exception exception("prefix:%s", longText.c_str());

	ASSERT_TRUE(exception.msg() != 0);
	ASSERT_TRUE(strncmp(exception.msg(), "prefix:", 7) == 0);
	ASSERT_TRUE(strlen(exception.msg()) < 4096);
}

int
main()
{
	TestSVGExceptionFormattingIsBounded();
	printf("import-export-svg-error-formatting: PASS\n");
	return 0;
}
