/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef NOTIFYING_LIST_H
#define NOTIFYING_LIST_H

#include <stdio.h>

#include <debugger.h>
#include <List.h>
#include <Message.h>

#include "AbstractLOAdapter.h"

template<typename ObjectType>
class NotifyingList {
public:

	class Listener {
	public:
								Listener() {}
		virtual					~Listener() {}

		virtual	void			ObjectAdded(const NotifyingList* list,
									ObjectType* object, int32 index) = 0;
		virtual	void			ObjectRemoved(const NotifyingList* list,
									ObjectType* object, int32 index) = 0;
	};


	class Observer : public Listener, public AbstractLOAdapter {
	public:
		enum {
			MSG_OBJECT_ADDED		= 'nloa',
			MSG_OBJECT_REMOVED		= 'nlor'
		};
	
		Observer(BHandler* handler)
			: Listener()
			, AbstractLOAdapter(handler)
		{
		}
	
		virtual ~Observer()
		{
		}
	
		virtual void ObjectAdded(const NotifyingList* list, ObjectType* object,
			int32 index)
		{
			BMessage message(MSG_OBJECT_ADDED);
			message.AddPointer("list", list);
			message.AddPointer("object", object);
			message.AddInt32("index", index);
			DeliverMessage(message);
		}
	
		virtual	void ObjectRemoved(const NotifyingList* list,
			ObjectType* object, int32 index)
		{
			BMessage message(MSG_OBJECT_REMOVED);
			message.AddPointer("list", list);
			message.AddPointer("object", object);
			message.AddInt32("index", index);
			DeliverMessage(message);
		}
	};


	NotifyingList()
		:
		fObjects(16),
		fListeners(2)
	{
	}

	virtual ~NotifyingList()
	{
		int32 count = fListeners.CountItems();
		if (count > 0) {
			debugger("~NotifyingList() - there are still listeners "
				"attached\n");
		}
		_MakeEmpty();
	}

	bool AddObject(ObjectType* object)
	{
		return AddObject(object, CountObjects());
	}

	bool AddObject(ObjectType* object, int32 index)
	{
		if (object == NULL)
			return false;

		// prevent adding the same object twice
		if (HasObject(object))
			return false;

		if (fObjects.AddItem((void*)object, index)) {
			object->AddReference();
			_NotifyObjectAdded(object, index);
			return true;
		}

		fprintf(stderr, "ObjectContainer::AddObject() - out of memory!\n");
		return false;
	}

	bool RemoveObject(ObjectType* object)
	{
		int32 index = fObjects.IndexOf(object);
		if (fObjects.RemoveItem(index)) {
			_NotifyObjectRemoved(object, index);
			object->RemoveReference();
			return true;
		}

		return false;
	}

	ObjectType* RemoveObject(int32 index)
	{
		ObjectType* object = reinterpret_cast<ObjectType*>(
			fObjects.RemoveItem(index));
		if (object) {
			_NotifyObjectRemoved(object, index);
			object->RemoveReference();
		}

		return object;
	}

	void MakeEmpty()
	{
		_MakeEmpty();
	}

	int32 CountObjects() const
	{
		return fObjects.CountItems();
	}

	bool HasObject(ObjectType* object) const
	{
		return fObjects.HasItem(object);
	}

	int32 IndexOf(ObjectType* object) const
	{
		return fObjects.IndexOf(object);
	}

	ObjectType* ObjectAt(int32 index) const
	{
		return reinterpret_cast<ObjectType*>(fObjects.ItemAt(index));
	}

	ObjectType* ObjectAtFast(int32 index) const
	{
		return reinterpret_cast<ObjectType*>(fObjects.ItemAtFast(index));
	}

	bool AddListener(Listener* listener)
	{
		if (listener && !fListeners.HasItem(listener))
			return fListeners.AddItem(listener);
		return false;
	}

	bool RemoveListener(Listener* listener)
	{
		return fListeners.RemoveItem(listener);
	}

private:
	void _MakeEmpty()
	{
		for (int32 i = CountObjects() - 1; i >= 0; i--) {
			ObjectType* object = ObjectAtFast(i);
			_NotifyObjectRemoved(object, i);
			object->RemoveReference();
		}
		fObjects.MakeEmpty();
	}

	void _NotifyObjectAdded(ObjectType* object, int32 index) const
	{
		BList listeners(fListeners);
		int32 count = listeners.CountItems();
		for (int32 i = 0; i < count; i++) {
			Listener* listener = reinterpret_cast<Listener*>(
				listeners.ItemAtFast(i));
			listener->ObjectAdded(this, object, index);
		}
	}

	void _NotifyObjectRemoved(ObjectType* object, int32 index) const
	{
		BList listeners(fListeners);
		int32 count = listeners.CountItems();
		for (int32 i = 0; i < count; i++) {
			Listener* listener = reinterpret_cast<Listener*>(
				listeners.ItemAtFast(i));
			listener->ObjectRemoved(this, object, index);
		}
	}

	BList fObjects;
	BList fListeners;
};

#endif	// NOTIFYING_LIST_H
