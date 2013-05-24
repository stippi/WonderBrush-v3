/*
 * Copyright 2004-2009, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#ifndef HASH_SET_HUGO_H
#define HASH_SET_HUGO_H

#include <new>

#include "OpenHashTableHugo.h"

#include "AutoLocker.h"
#include <Locker.h>


// HashSetElement
template<typename Key>
class HashSetElement : public HashTableLink<HashSetElement<Key> > {
private:
	typedef HashSetElement<Key> Element;

public:
	HashSetElement()
		:
		fKey()
	{
	}

	HashSetElement(const Key& key)
		:
		fKey(key)
	{
	}

	Key		fKey;
};


// HashSetTableDefinition
template<typename Key>
struct HashSetTableDefinition {
	typedef Key					KeyType;
	typedef	HashSetElement<Key>	ValueType;

	size_t HashKey(const KeyType& key) const
		{ return key.GetHashCode(); }
	size_t Hash(const ValueType* value) const
		{ return HashKey(value->fKey); }
	bool Compare(const KeyType& key, const ValueType* value) const
		{ return value->fKey == key; }
	HashTableLink<ValueType>* GetLink(ValueType* value) const
		{ return value; }
};


// HashSet
template<typename Key>
class HashSet {
protected:
	typedef OpenHashTable<HashSetTableDefinition<Key> > ElementTable;
	typedef typename ElementTable::Iterator TableIterator;
	typedef HashSetElement<Key>	Element;

public:
	class Iterator {
	public:
		Iterator(const Iterator& other)
			:
			fSet(other.fSet),
			fIterator(other.fIterator),
			fElement(other.fElement)
		{
		}

		bool HasNext() const
		{
			return fIterator.HasNext();
		}

		Key Next()
		{
			fElement = fIterator.Next();
			if (fElement == NULL)
				return Key();

			return fElement->fKey;
		}

		bool Remove()
		{
			if (fElement == NULL)
				return false;

			fSet->fTable.RemoveUnchecked(fElement);
			delete fElement;
			fElement = NULL;

			return true;
		}

		Iterator& operator=(const Iterator& other)
		{
			fSet = other.fSet;
			fIterator = other.fIterator;
			fElement = other.fElement;
			return *this;
		}

	private:
		Iterator(HashSet<Key>* set)
			:
			fSet(set),
			fIterator(set->fTable.GetIterator()),
			fElement(NULL)
		{
		}

	private:
		HashSet<Key>*					fSet;
		TableIterator					fIterator;
		Element*						fElement;

	private:
		friend class HashSet<Key>;
	};

	HashSet();
	HashSet(const HashSet<Key>& other);
	~HashSet();

	status_t InitCheck() const;

	HashSet<Key>& operator=(const HashSet<Key>& other);
	bool operator==(const HashSet<Key>& other) const;
	bool operator!=(const HashSet<Key>& other) const;

	status_t Add(const Key& key);
	bool Remove(const Key& key);
	void Clear();
	bool Contains(const Key& key) const;

	int32 Size() const;

	Iterator GetIterator();

protected:
	friend class Iterator;

protected:
	ElementTable	fTable;
};


// SynchronizedHashSet
template<typename Key>
class SynchronizedHashSet : public BLocker {
public:
	typedef typename HashSet<Key>::Iterator Iterator;

	SynchronizedHashSet() : BLocker("synchronized hash set")	{}
	~SynchronizedHashSet()	{ Lock(); }

	status_t InitCheck() const
	{
		return fSet.InitCheck();
	}

	status_t Add(const Key& key)
	{
		MapLocker locker(this);
		if (!locker.IsLocked())
			return B_ERROR;
		return fSet.Add(key);
	}

	bool Remove(const Key& key)
	{
		MapLocker locker(this);
		if (!locker.IsLocked())
			return false;
		return fSet.Remove(key);
	}

	void Clear()
	{
		MapLocker locker(this);
		fSet.Clear();
	}

	bool Contains(const Key& key) const
	{
		const BLocker* lock = this;
		MapLocker locker(const_cast<BLocker*>(lock));
		if (!locker.IsLocked())
			return false;
		return fSet.Contains(key);
	}

	int32 Size() const
	{
		const BLocker* lock = this;
		MapLocker locker(const_cast<BLocker*>(lock));
		return fSet.Size();
	}

	Iterator GetIterator()
	{
		return fSet.GetIterator();
	}

	// for debugging only
	const HashSet<Key>& GetUnsynchronizedSet() const	{ return fSet; }
	HashSet<Key>& GetUnsynchronizedSet()				{ return fSet; }

protected:
	typedef AutoLocker<BLocker> MapLocker;

	HashSet<Key>	fSet;
};


// HashSet

// constructor
template<typename Key>
HashSet<Key>::HashSet()
	:
	fTable()
{
	fTable.Init();
}


// constructor
template<typename Key>
HashSet<Key>::HashSet(const HashSet<Key>& other)
	:
	fTable()
{
	fTable.Init();
	*this = other;
}


// destructor
template<typename Key>
HashSet<Key>::~HashSet()
{
	Clear();
}


// InitCheck
template<typename Key>
status_t
HashSet<Key>::InitCheck() const
{
	return (fTable.TableSize() > 0 ? B_OK : B_NO_MEMORY);
}


// operator=
template<typename Key>
HashSet<Key>&
HashSet<Key>::operator=(const HashSet<Key>& other)
{
	Clear();
	HashSet<Key>::Iterator iterator = const_cast<HashSet&>(other).GetIterator();
	while (iterator.HasNext()) {
		Add(iterator.Next());
	}
	return *this;
}


// operator==
template<typename Key>
bool
HashSet<Key>::operator==(const HashSet<Key>& other) const
{
	if (Size() != other.Size())
		return false;
	
	HashSet<Key>::Iterator thisIterator = GetIterator();
	HashSet<Key>::Iterator otherIterator
		= const_cast<HashSet&>(other).GetIterator();
	while (thisIterator.HasNext() && otherIterator.HasNext()) {
		if (thisIterator.Next() != otherIterator.Next())
			return false;
	}

	return true;
}


// operator!=
template<typename Key>
bool
HashSet<Key>::operator!=(const HashSet<Key>& other) const
{
	return !(*this == other);
}


// Add
template<typename Key>
status_t
HashSet<Key>::Add(const Key& key)
{
	Element* element = fTable.Lookup(key);
	if (element) {
		// already contains the value
		return B_OK;
	}

	// does not contain the key yet: create an element and add it
	element = new(std::nothrow) Element(key);
	if (!element)
		return B_NO_MEMORY;

	status_t error = fTable.Insert(element);
	if (error != B_OK)
		delete element;

	return error;
}


// Remove
template<typename Key>
bool
HashSet<Key>::Remove(const Key& key)
{
	Element* element = fTable.Lookup(key);
	if (element == NULL)
		return false;

	fTable.Remove(element);
	delete element;

	return true;
}


// Clear
template<typename Key>
void
HashSet<Key>::Clear()
{
	// clear the table and delete the elements
	Element* element = fTable.Clear(true);
	while (element != NULL) {
		Element* next = element->fNext;
		delete element;
		element = next;
	}
}


// Contains
template<typename Key>
bool
HashSet<Key>::Contains(const Key& key) const
{
	return fTable.Lookup(key) != NULL;
}


// Size
template<typename Key>
int32
HashSet<Key>::Size() const
{
	return fTable.CountElements();
}

// GetIterator
template<typename Key>
typename HashSet<Key>::Iterator
HashSet<Key>::GetIterator()
{
	return Iterator(this);
}


#endif	// HASH_SET_HUGO_H
