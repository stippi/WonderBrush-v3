/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>. All rights reserved.
 */
#ifndef BOUNDED_OBJECT_H
#define BOUNDED_OBJECT_H

#include "Object.h"

class BoundedObject : public Object {
public:
								BoundedObject();
								BoundedObject(const BoundedObject& other);
	virtual						~BoundedObject();

	// Object interface
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);
	virtual	void				TransformationChanged();

	// BoundedObject
	virtual	BRect				Bounds() = 0;
	inline	BRect				TransformedBounds() const
									{ return fTransformedBounds; }

			void				InitBounds();
			void				UpdateBounds();

			void				SetOpacity(uint8 opacity);
	inline	uint8				Opacity() const
									{ return fOpacity; }

			void				NotifyAndUpdate();

private:
			uint8				fOpacity;
			BRect				fTransformedBounds;
};

#endif // BOUNDED_OBJECT_H
