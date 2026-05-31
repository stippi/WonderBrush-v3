#ifndef WB_TEST_STUB_STRING_H
#define WB_TEST_STUB_STRING_H

#include <string>

class BString {
public:
	BString() {}
	BString(const char* string)
		: fString(string != 0 ? string : "")
	{
	}
	BString(const BString& other)
		: fString(other.fString)
	{
	}
	BString& operator=(const char* string)
	{
		fString = string != 0 ? string : "";
		return *this;
	}
	BString& operator=(const BString& other)
	{
		fString = other.fString;
		return *this;
	}
	BString& operator<<(const char* string)
	{
		fString += string != 0 ? string : "";
		return *this;
	}
	bool operator==(const char* string) const
	{
		return fString == (string != 0 ? string : "");
	}
	int Length() const { return (int)fString.size(); }
	const char* String() const { return fString.c_str(); }

private:
	std::string fString;
};

#endif // WB_TEST_STUB_STRING_H
