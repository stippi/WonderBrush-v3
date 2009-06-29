/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef SHARED_OBJECT_CACHE_H
#define SHARED_OBJECT_CACHE_H

#include "OpenHashTableHugo.h"

template<typename ObjectType> class SharedObjectCache;

template<typename ObjectType>
class SharedObject : public ObjectType {
public:
	typedef SharedObject<ObjectType>			SharedObjectType;
	typedef SharedObjectCache<ObjectType>		CacheType;
	typedef HashTableLink<SharedObjectType>		LinkType;

public:
	SharedObject()
		:
		ObjectType(),
		fCache(NULL)
	{
	}

	SharedObject(const ObjectType& object)
		:
		ObjectType(object),
		fCache(NULL)
	{
	}

	SharedObject(const SharedObject& object)
		:
		ObjectType(object),
		fCache(NULL)
	{
	}

	virtual ~SharedObject()
	{
		if (fCache != NULL)
			debugger("SharedObject still in a cache!");
	}

	SharedObject& operator=(const ObjectType& object)
	{
		ObjectType::operator=(object);
		return *this;
	}

	SharedObject& operator=(const SharedObject& object)
	{
		ObjectType::operator=(object);
		// Do not copy the fCache member!
		return *this;
	}

	bool operator==(const ObjectType& object) const
	{
		return ObjectType::operator==(object);
	}

	bool operator==(const SharedObject& object) const
	{
		return ObjectType::operator==(object);
	}

	void SetCache(CacheType* cache)
	{
		if (fCache != NULL && fCache != cache)
			debugger("SharedObject already in another cache!");
		fCache = cache;
	}

	CacheType* Cache() const
	{
		return fCache;
	}

	LinkType* SharedObjectCacheLink()
	{
		return &fCacheLink;
	}

private:
	LinkType	fCacheLink;
	CacheType*	fCache;
};

template<typename ValueT>
struct SharedObjectCacheHashDefinition {
	typedef ValueT							ValueType;
//	typedef ValueType::KeyType				KeyType;
	typedef ValueT							KeyType;

	size_t HashKey(const KeyType& key) const
	{
		return key.HashKey();
	}

	size_t Hash(ValueType* value) const
	{
		return value->HashKey();
	}

	bool Compare(const KeyType& key, ValueType* value) const
	{
		return *value == key;
	}

	HashTableLink<ValueType>* GetLink(ValueType* value) const
	{
		return value->SharedObjectCacheLink();
	}
};


template<typename ObjectType>
class SharedObjectCache {
public:
	typedef SharedObject<ObjectType>	SharedObjectType;
private:
	typedef OpenHashTable<SharedObjectCacheHashDefinition<SharedObjectType> >
		HashTable;
	typedef ObjectType	KeyType;
public:
	SharedObjectCache()
	{
		fTable.Init();
	}

	SharedObjectType* Get(const KeyType& key)
	{
		SharedObjectType* object = fTable.Lookup(key);
		if (object != NULL) {
			object->AddReference();
			return object;
		}
		// Shared entry for this value/key does not yet exist.
		object = new(std::nothrow) SharedObjectType(key);
		if (object == NULL || Insert(object) != B_OK) {
			delete object;
			return NULL;
		}
		return object;
	}

	bool Put(const KeyType& key)
	{
		SharedObjectType* object = fTable.Lookup(key);
		if (object != NULL) {
			if (object->CountReferences() == 1)
				Remove(object);
			object->RemoveReference();
			return true;
		}
		return false;
	}

	SharedObjectType* PrepareForModifications(SharedObjectType* object)
	{
		if (object->Cache() != this)
			debugger("PrepareForModifications(): Object not in cache");

		// Returns an object that is not part of the table.
		if (object->CountReferences() == 1) {
			Remove(object);
			return object;
		}

		// The caller loses it's reference to the object he passed to
		// this method.
		object->RemoveReference();
		return new(std::nothrow) SharedObjectType(*object);
	}

	SharedObjectType* CommitModifications(SharedObjectType* object)
	{
		if (object->Cache() == this)
			debugger("CommitModifications(): Trying to insert object twice");
		if (object->Cache() != NULL) {
			debugger("CommitModifications(): Trying to insert object which "
				"belongs to another cache");
		}

		// Returns an object of the same value that is in the cache.
		// If such an object was not already in the cache, the passed
		// object is inserted.
		SharedObjectType* cachedObject = fTable.Lookup(*object);
		if (cachedObject == NULL) {
			if (Insert(object) == B_OK)
				return object;
			// Return NULL on error.
			return NULL;
		}
		// We switched to the already cached object, transfer the reference
		// from the object to the cached object.
		cachedObject->AddReference();
		object->RemoveReference();
		return cachedObject;
	}

	SharedObjectType* Lookup(const KeyType& key) const
	{
		return fTable.Lookup(key);
	}

	status_t Insert(SharedObjectType* object)
	{
		if (object->Cache() == this)
			debugger("Insert(): Trying to insert object twice");
		if (object->Cache() != NULL) {
			debugger("Insert(): Trying to insert object which belongs to "
				"another cache");
		}

		status_t ret = fTable.Insert(object);
		if (ret == B_OK)
			object->SetCache(this);
		return ret;
	}

	void Remove(SharedObjectType* object)
	{
		fTable.Remove(object);
		object->SetCache(NULL);
	}

private:
	HashTable	fTable;
};

#endif // SHARED_OBJECT_CACHE_H
