#ifndef WB_TEST_STUB_PROPERTY_OBJECT_H
#define WB_TEST_STUB_PROPERTY_OBJECT_H

#include <SupportDefs.h>

#include "Property.h"

class PropertyObject {
public:
	bool AddProperty(Property*) { return true; }
	int32 Value(uint32, int32 defaultValue) const { return defaultValue; }
	Property* FindProperty(uint32) const { return 0; }
};

#endif // WB_TEST_STUB_PROPERTY_OBJECT_H
