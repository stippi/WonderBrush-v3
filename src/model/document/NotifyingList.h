/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef NOTIFYING_LIST_H
#define NOTIFYING_LIST_H


#include <stdio.h>

#include <debugger.h>
#include <List.h>


template<typename ObjectType>
class NotifyingList {
public:

	class Listener {
	 public:
								Listener() {}
		virtual					~Listener() {}

		virtual	void			ObjectAdded(ObjectType* object,
									int32 index) = 0;
		virtual	void			ObjectRemoved(ObjectType* object) = 0;
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
		if (fObjects.RemoveItem((void*)object)) {
			_NotifyObjectRemoved(object);
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
			_NotifyObjectRemoved(object);
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
		if (listener && !fListeners.HasItem((void*)listener))
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
			_NotifyObjectRemoved(object);
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
			listener->ObjectAdded(object, index);
		}
	}

	void _NotifyObjectRemoved(ObjectType* object) const
	{
		BList listeners(fListeners);
		int32 count = listeners.CountItems();
		for (int32 i = 0; i < count; i++) {
			Listener* listener = reinterpret_cast<Listener*>(
				listeners.ItemAtFast(i));
			listener->ObjectRemoved(object);
		}
	}

	BList fObjects;
	BList fListeners;
};

#endif	// NOTIFYING_LIST_H
