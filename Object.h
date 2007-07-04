/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef OBJECT_H
#define OBJECT_H

#include <Rect.h>

class Layer;
class ObjectSnapshot;

class Object {
 public:
								Object();
	virtual						~Object();

	virtual	ObjectSnapshot*		Snapshot() const = 0;

	virtual	void				SetParent(Layer* layer);
	inline	Layer*				Parent() const
									{ return fParent; }

	virtual	void				ExtendDirtyArea(BRect& area) const;

			void				UpdateChangeCounter();
	inline	uint32				ChangeCounter() const
									{ return fChangeCounter; }

 private:
			uint32				fChangeCounter;
			Layer*				fParent;
};

#endif // OBJECT_H
