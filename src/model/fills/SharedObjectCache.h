/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef SHARED_OBJECT_CACHE_H
#define SHARED_OBJECT_CACHE_H

#include "OpenHashTableHugo.h"

template<typename SharedObjectType> class SharedObjectCache;

template<typename ObjectType>
class SharedObject : public ObjectType {
public:
	typedef SharedObject<ObjectType>			SharedObjectType;
	typedef SharedObjectCache<SharedObjectType>	CacheType;
	typedef HashTableLink<SharedObject>			LinkType;
	typedef ObjectType							KeyType;

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

	virtual ~SharedObject()
	{
	}

	void SetCache(CacheType* cache)
	{
		if (fCache != NULL && fCache != cache)
			debugger("SharedObject alread in another cache!");
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
	CacheType	fCache;
};

template<typename ValueT>
struct SharedObjectCacheHashDefinition {
	typedef ValueT							ValueType;
//	typedef typename ValueType::KeyType		KeyType;
	typedef ValueT							KeyType;

	size_t HashKey(const KeyType& key) const
	{
//		return ValueType::KeyHashValue(key);
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


template<typename SharedObjectType>
class SharedObjectCache {
private:
	typedef OpenHashTable<SharedObjectCacheHashDefinition<SharedObjectType> >
		HashTable;
	typedef typename SharedObjectType::KeyType	KeyType;
public:
	SharedObjectCache()
	{
		fTable.Init();
	}

	bool Get(const KeyType& key)
	{
		SharedObjectType* object = fTable->Lookup(key);
		if (object != NULL) {
			object->AddReference();
			return true;
		}
		return false;
	}

	bool Put(const KeyType& key)
	{
		SharedObjectType* object = fTable->Lookup(key);
		if (object != NULL) {
			if (object->ReferenceCount() == 1)
				Remove(object);
			object->RemoveReference();
			return true;
		}
		return false;
	}

	SharedObjectType* PrepareForModifications(SharedObjectType* object)
	{
		// Returns an object that is not part of the table.
		// TODO: ...
		return object;
	}

	SharedObjectType* CommitModifications(SharedObjectType* object)
	{
		// Returns an object that is not part of the table.
		// TODO: ...
		return object;
	}

	status_t Insert(SharedObjectType* object)
	{
		if (object->Cache() == this)
			debugger("Trying to insert object twice");
		if (object->Cache() != NULL)
			debugger("Trying to insert object which belongs to another cache");

		status_t ret = Insert(object->SharedObjectCacheLink());
		if (ret == B_OK)
			object->SetCache(this);
		return ret;
	}

private:
	HashTable	fTable;
};

#endif // SHARED_OBJECT_CACHE_H
