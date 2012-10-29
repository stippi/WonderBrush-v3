/*
 * Copyright 2007 - 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef OBJECT_H
#define OBJECT_H

#include <Rect.h>
#include <String.h>

#include "BaseObject.h"
#include "Transformable.h"


class BBitmap;
class Layer;
class ObjectSnapshot;


class Object : public Transformable, public BaseObject {
public:
								Object();
	virtual						~Object();

	virtual	ObjectSnapshot*		Snapshot() const = 0;

	virtual	void				SetParent(Layer* layer);
	inline	Layer*				Parent() const
									{ return fParent; }
			int32				Level() const;

	virtual	bool				GetIcon(const BBitmap* bitmap) const;

	virtual	Transformable		Transformation() const;
			Transformable		LocalTransformation() const
									{ return *this; }
	virtual	bool				IsRegularTransformable() const;

	virtual	void				ExtendDirtyArea(BRect& area) const;

			void				InvalidateParent(const BRect& area);
			void				InvalidateParent();

	virtual	bool				HitTest(const BPoint& canvasPoint);

			void				UpdateChangeCounter();
	inline	uint32				ChangeCounter() const
									{ return fChangeCounter; }

protected:
	// BaseObject interface
	virtual void				NotifyListeners();

	// Transformable interface
	virtual	void				TransformationChanged();

private:
			uint32				fChangeCounter;
			Layer*				fParent;
};

#endif // OBJECT_H
