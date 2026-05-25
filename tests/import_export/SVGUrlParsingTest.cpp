#include "Test.h"

#include <fstream>
#include <sstream>
#include <string>

static std::string
ReadFile(const char* path)
{
	std::ifstream file(path);
	ASSERT_TRUE(file.good());
	std::ostringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

static void
TestSVGUrlParsingDoesNotScanPastStringEnd()
{
	std::string parser = ReadFile("../src/import_export/svg/SVGParser.cpp");

	ASSERT_TRUE(parser.find("while (*begin != '#')") == std::string::npos);
	ASSERT_TRUE(parser.find("while (*end != ')')") == std::string::npos);
	ASSERT_TRUE(parser.find("parse_url(value)") == std::string::npos);
}

static void
TestSVGUrlStorageUsesBoundedCopies()
{
	std::string builder = ReadFile("../src/import_export/svg/DocumentBuilder.cpp");
	std::string builderHeader = ReadFile("../src/import_export/svg/DocumentBuilder.h");

	ASSERT_TRUE(builder.find("sprintf(cur_attr().fill_url") == std::string::npos);
	ASSERT_TRUE(builder.find("sprintf(cur_attr().stroke_url") == std::string::npos);
	ASSERT_TRUE(builderHeader.find("sprintf(stroke_url") == std::string::npos);
	ASSERT_TRUE(builderHeader.find("sprintf(fill_url") == std::string::npos);
}

int
main()
{
	TestSVGUrlParsingDoesNotScanPastStringEnd();
	TestSVGUrlStorageUsesBoundedCopies();
	printf("import-export-svg-url-parsing: PASS\n");
	return 0;
}
