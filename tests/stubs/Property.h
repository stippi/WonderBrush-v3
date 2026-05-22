#ifndef WB_TEST_STUB_PROPERTY_H
#define WB_TEST_STUB_PROPERTY_H

#include <SupportDefs.h>

class PropertyObject;
class BString;

class Property {
public:
	Property(uint32 = 0) {}
	virtual ~Property() {}
};

class IntProperty : public Property {
public:
	IntProperty(uint32 identifier, int32, int32, int32)
		: Property(identifier)
	{
	}
};

#endif // WB_TEST_STUB_PROPERTY_H
