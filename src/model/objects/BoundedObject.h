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

	virtual	BRect				Bounds() = 0;
	inline	BRect				TransformedBounds() const
									{ return fTransformedBounds; }

	virtual	void				TransformationChanged();

			void				InitBounds();
			void				UpdateBounds();

			void				SetOpacity(float opacity);
	inline	float				Opacity() const
									{ return fOpacity; }

			void				NotifyAndUpdate();

private:
			float				fOpacity;
			BRect				fTransformedBounds;
};

#endif // BOUNDED_OBJECT_H
