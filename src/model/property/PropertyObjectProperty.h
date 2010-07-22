/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef PROPERTY_OBJECT_PROPERTY_H
#define PROPERTY_OBJECT_PROPERTY_H

#include "Property.h"
#include "PropertyObject.h"

class PropertyObjectProperty : public Property {
public:
								PropertyObjectProperty(uint32 identifier);
								PropertyObjectProperty(
									const PropertyObjectProperty& other);
								PropertyObjectProperty(BMessage* archive);
	virtual						~PropertyObjectProperty();

	// BArchivable interface
	virtual	status_t			Archive(BMessage* archive,
										bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	// Property interface
	virtual	Property*			Clone() const;

	virtual	type_code			Type() const
									{ return B_ANY_TYPE; }

	virtual	bool				SetValue(const char* value);
	virtual	bool				SetValue(const Property* other);
	virtual	void				GetValue(BString& string);

	virtual	bool				InterpolateTo(const Property* other,
									float scale);

	// PropertyObjectProperty
			PropertyObject&		Value()
									{ return fValue; }

private:
			PropertyObject		fValue;
};


#endif // PROPERTY_OBJECT_PROPERTY_H


