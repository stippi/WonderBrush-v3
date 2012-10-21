/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#ifndef LAYER_H
#define LAYER_H

#include <List.h>
#include <Rect.h>

#include "BlendingMode.h"
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

				void			SuspendUpdates(bool suspend);
				bool			UpdatesEnabled() const;
		virtual	void			AreaInvalidated(Layer* layer,
									const BRect& area);
		virtual	void			AllAreasInvalidated();

		virtual	void			ListenerAttached(Layer* layer);

	private:
				int32			fUpdatesSuspended;
	};

								Layer(const BRect& bounds);
	virtual						~Layer();

	// BaseObject interface
	virtual	status_t			Unarchive(const BMessage* archive);
	virtual status_t			Archive(BMessage* into,
										bool deep = true) const;

	virtual	const char*			DefaultName() const;
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);

	virtual	void				TransformationChanged();

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	bool				HitTest(const BPoint& canvasPoint);

	// Layer
			bool				AddObject(Object* object);
			bool				AddObject(Object* object, int32 index);
			Object*				RemoveObject(int32 index);
			bool				RemoveObject(Object* object);
			Object*				ObjectAt(int32 index) const;
			Object*				ObjectAtFast(int32 index) const;
			int32				IndexOf(Object* object) const;
			int32				CountObjects() const;
			bool				HasObject(Object* object) const;

			void				SuspendUpdates(bool suspend);
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

			void				SetGlobalAlpha(uint8 globalAlpha);
			uint8				GlobalAlpha() const
									{ return fGlobalAlpha; }

			void				SetBlendingMode(::BlendingMode blendingMode);
			::BlendingMode		BlendingMode() const
									{ return fBlendingMode; }

private:
			BRect				fBounds;
			uint8				fGlobalAlpha;
			::BlendingMode		fBlendingMode;

			BList				fObjects;
			BList				fListeners;
};

#endif // LAYER_H
