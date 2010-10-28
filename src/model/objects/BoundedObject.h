/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>. All rights reserved.
 */
#ifndef BOUNDED_OBJECT_H
#define BOUNDED_OBJECT_H

#include "Object.h"

class BoundedObject : public Object {
public:
								BoundedObject();
	virtual						~BoundedObject();

	virtual	BRect				Bounds() = 0;
	inline	BRect				TransformedBounds() const
									{ return fTransformedBounds; }

	virtual	void				TransformationChanged();

			void				InitBounds();
			void				UpdateBounds();

			void				NotifyAndUpdate();

private:
			BRect				fTransformedBounds;
};

#endif // BOUNDED_OBJECT_H
