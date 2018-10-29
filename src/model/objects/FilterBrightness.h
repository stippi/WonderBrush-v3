/*
 * Copyright 2018, Stephan Aßmus <superstippi@gmx.de>.
 * Distributed under the terms of the MIT License.
 */
#ifndef FILTER_BRIGHTNESS_H
#define FILTER_BRIGHTNESS_H

#include "Object.h"

class FilterBrightness : public Object {
public:
								FilterBrightness();
								FilterBrightness(const FilterBrightness& other);
								FilterBrightness(int32 offset, float factor);
	virtual						~FilterBrightness();

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

	// FilterBrightness
			void				SetOffset(int32 offset);
			int32				Offset() const
									{ return fOffset; }

			void				SetFactor(float factor);
			float				Factor() const
									{ return fFactor; }

private:
 			int32				fOffset;
 			float				fFactor;
};

#endif // FILTER_BRIGHTNESS_H
