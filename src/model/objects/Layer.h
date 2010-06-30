/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef LAYER_H
#define LAYER_H


#include <List.h>
#include <Rect.h>

#include "Object.h"


class Layer : public Object {
 public:
	class Listener {
	 public:
								Listener();
		virtual					~Listener();

		virtual	void			ObjectAdded(Layer* layer, Object* object,
									int32 index);
		virtual	void			ObjectRemoved(Layer* layer, Object* object,
									int32 index);
		virtual	void			ObjectChanged(Layer* layer, Object* object,
									int32 index);

		virtual	void			AreaInvalidated(Layer* layer,
									const BRect& area);

		virtual	void			ListenerAttached(Layer* layer);
	};

								Layer(const BRect& bounds);
	virtual						~Layer();

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	const char*			DefaultName() const;

	virtual	bool				HitTest(const BPoint& canvasPoint) const;

	// Layer
			bool				AddObject(Object* object);
			bool				AddObject(Object* object, int32 index);
			Object*				RemoveObject(int32 index);
			Object*				ObjectAt(int32 index) const;
			Object*				ObjectAtFast(int32 index) const;
			int32				IndexOf(Object* object) const;
			int32				CountObjects() const;
			bool				HasObject(Object* object) const;

			void				Invalidate(const BRect& area,
									int32 objectIndex = 0);
			void				ObjectChanged(Object* object);

			bool				HitTest(const BPoint& canvasPoint,
									Layer** layer, Object** object,
									bool recursive) const;

			bool				AddListener(Listener* listener);
			void				RemoveListener(Listener* listener);

	static	bool				AddListenerRecursive(Layer* layer,
									Listener* listener);
	static	void				RemoveListenerRecursive(Layer* layer,
									Listener* listener);

			BRect				Bounds() const
									{ return fBounds; }
 private:
			BRect				fBounds;

			BList				fObjects;
			BList				fListeners;
};

#endif // LAYER_H
