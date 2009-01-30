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
#include <String.h>

#include "Referenceable.h"
#include "Transformable.h"


class BBitmap;
class Layer;
class ObjectSnapshot;


class Object : public Transformable, public Referenceable {
 public:
								Object();
	virtual						~Object();

	virtual	ObjectSnapshot*		Snapshot() const = 0;

	virtual	void				SetParent(Layer* layer);
	inline	Layer*				Parent() const
									{ return fParent; }
			int32				Level() const;

	virtual	const char*			DefaultName() const = 0;
			void				SetName(const char* name);
			const char*			Name() const;

	virtual	bool				GetIcon(const BBitmap* bitmap) const;

	virtual	Transformable		Transformation() const;
			Transformable		LocalTransformation() const
									{ return *this; }
	virtual	bool				IsRegularTransformable() const;

	virtual	void				ExtendDirtyArea(BRect& area) const;

			void				InvalidateParent(const BRect& area);

			void				UpdateChangeCounter();
	inline	uint32				ChangeCounter() const
									{ return fChangeCounter; }

 private:
			uint32				fChangeCounter;
			Layer*				fParent;
			BString				fName;
};

#endif // OBJECT_H
