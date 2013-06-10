/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * Distributed under the terms of the MIT License.
 */
#ifndef FILTER_SATURATION_H
#define FILTER_SATURATION_H

#include "Object.h"

class FilterSaturation : public Object {
public:
								FilterSaturation();
								FilterSaturation(float saturation);
	virtual						~FilterSaturation();

	// BaseObject interface
	virtual	const char*			DefaultName() const;
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	bool				IsRegularTransformable() const;

	// FilterSaturation
			void				SetSaturation(float saturation);
			float				Saturation() const
									{ return fSaturation; }

private:
 			float				fSaturation;
};

#endif // FILTER_SATURATION_H
