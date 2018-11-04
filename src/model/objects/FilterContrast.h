/*
 * Copyright 2018, Stephan Aßmus <superstippi@gmx.de>.
 * Distributed under the terms of the MIT License.
 */
#ifndef FILTER_CONTRAST_H
#define FILTER_CONTRAST_H

#include "Object.h"

class FilterContrast : public Object {
public:
								FilterContrast();
								FilterContrast(const FilterContrast& other);
								FilterContrast(float contrast, uint8 center);
	virtual						~FilterContrast();

	// BaseObject interface
	virtual	BaseObject*			Clone(CloneContext& context) const;
	virtual	const char*			DefaultName() const;
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	bool				IsRegularTransformable() const;

	// FilterContrast
			void				SetContrast(float contrast);
			float				Contrast() const
									{ return fContrast; }
			void				SetCenter(uint8 center);
			uint8				Center() const
									{ return fCenter; }

private:
 			float				fContrast;
 			uint8				fCenter;
};

#endif // FILTER_CONTRAST_H
