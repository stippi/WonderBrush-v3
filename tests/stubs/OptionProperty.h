#ifndef WB_TEST_STUB_OPTION_PROPERTY_H
#define WB_TEST_STUB_OPTION_PROPERTY_H

#include "Property.h"

class OptionProperty : public Property {
public:
	OptionProperty(uint32 identifier)
		: Property(identifier)
		, fCurrentOptionID(0)
	{
	}

	void AddOption(int32, const char*) {}
	int32 CurrentOptionID() const { return fCurrentOptionID; }
	bool SetCurrentOptionID(int32 id)
	{
		fCurrentOptionID = id;
		return true;
	}

private:
	int32 fCurrentOptionID;
};

#endif // WB_TEST_STUB_OPTION_PROPERTY_H
