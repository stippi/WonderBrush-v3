/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "OpenHashTableHugo.h"


template<typename HashMapKeyType, typename HashMapValueType>
struct HashMapLink : HashTableLink<HashMapLink<HashMapKeyType,
	HashMapValueType > > {
	typedef HashMapKeyType KeyType;

	HashMapLink(const HashMapKeyType& key, const HashMapValueType& value)
		:
		Key(key),
		Value(value)
	{
	}

	HashMapLink(const HashMapLink& other)
		:
		Key(other.Key),
		Value(other.Value)
	{
	}

	HashMapLink& operator=(const HashMapLink& other)
	{
		Key = other.Key;
		Value = other.Value;
		return *this;
	}

	HashMapKeyType	 Key;
	HashMapValueType Value;
};


template<typename HashMapKeyType, typename HashMapValueType>
struct HashMapDefinition {
	typedef HashMapLink<HashMapKeyType, HashMapValueType>	ValueType;
	typedef typename ValueType::KeyType						KeyType;

	size_t HashKey(const KeyType& key) const
	{
		return key.HashKey();
	}

	size_t Hash(ValueType* link) const
	{
		return link->Key.HashKey();
	}

	bool Compare(const KeyType& key, ValueType* link) const
	{
		return link->Key == key;
	}

	HashTableLink<ValueType>* GetLink(ValueType* link) const
	{
		return link;
	}
};


template<typename HashMapKeyType, typename HashMapValueType>
class HashMap : public OpenHashTable<HashMapDefinition<HashMapKeyType,
	HashMapValueType> > {
private:
	typedef OpenHashTable<HashMapDefinition<HashMapKeyType,
		HashMapValueType> > HashTableType;

public:
	typedef HashMapLink<HashMapKeyType, HashMapValueType> LinkType;

	HashMap()
	{
	}

	~HashMap()
	{
		Clear();
	}

	bool ContainsKey(const HashMapKeyType& key) const
	{
		return Lookup(key) != NULL;
	}

	HashMapValueType Get(const HashMapKeyType& key) const
	{
		LinkType* link = Lookup(key);
		if (link != NULL)
			return link->Value;

		return HashMapValueType();
	}

	status_t Put(const HashMapKeyType& key, const HashMapValueType& value)
	{
		LinkType* link = NULL;
		try {
			link = new LinkType(key, value);
			if (Insert(link) == B_OK)
				return B_OK;
		} catch (std::bad_alloc) {
		}

		delete link;
		return B_NO_MEMORY;
	}

	bool RemoveKey(const HashMapKeyType& key)
	{
		LinkType* link = Lookup(key);
		if (link != NULL && Remove(link)) {
			delete link;
			return true;
		}
		return false;
	}

	// Needs to look up the entry by the value, so the operation depends
	// on how many entries are already in the HashMap!
	bool RemoveValue(const HashMapValueType& value)
	{
		typename HashTableType::Iterator iterator = GetIterator();
		while (iterator.HasNext()) {
			LinkType* link = iterator.Next();
			if (link->Value == value) {
				bool removed = Remove(link);
				delete link;
				return removed;
			}
		}
		return false;
	}

	void Clear()
	{
		typename HashTableType::Iterator iterator(this);
		while (LinkType* link = iterator.Next()) {
			RemoveUnchecked(link);
			delete link;
		}
	}
};


// Convenience HashMapKeyType definitions

template<typename Value>
struct HashKey32 {
	HashKey32()
		:
		value(0)
	{
	}

	HashKey32(const Value& value)
		:
		value(value)
	{
	}

	HashKey32(const HashKey32<Value>& other)
		:
		value(other.value)
	{
	}

	HashKey32<Value>& operator=(const HashKey32<Value>& other)
	{
		value = other.value;
		return *this;
	}

	bool operator==(const HashKey32<Value>& other) const
	{
		return (value == other.value);
	}

	size_t HashKey() const
	{
		return (size_t)value;
	}

	Value	value;
};


template<typename Value>
struct HashKey64 {
	HashKey64()
		:
		value(0)
	{
	}

	HashKey64(const Value& value)
		:
		value(value)
	{
	}

	HashKey64(const HashKey64<Value>& other)
		:
		value(other.value)
	{
	}

	HashKey64<Value>& operator=(const HashKey64<Value>& other)
	{
		value = other.value;
		return *this;
	}

	bool operator==(const HashKey64<Value>& other) const
	{
		return (value == other.value);
	}

	size_t HashKey() const
	{
		uint64 v = (uint64)value;
		return (size_t)(v >> 32) ^ (size_t)v;
	}

	Value	value;
};


#endif // HASH_MAP_H
