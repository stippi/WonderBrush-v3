/*
 * Copyright (c) 2009, Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef OBJECT_CACHE_H
#define OBJECT_CACHE_H

#include <new>
#include <stdlib.h>

#include <SupportDefs.h>

template <typename ObjectType, bool PlainOldData, uint32 BlockSize = 8>
class ObjectCache {
	typedef ObjectCache<ObjectType, PlainOldData, BlockSize> SelfType;
public:
	ObjectCache()
		:
		fItems(NULL),
		fCount(0),
		fAllocatedCount(0)
	{
	}

	ObjectCache(const SelfType& other)
		:
		fItems(NULL),
		fCount(0),
		fAllocatedCount(0)
	{
		*this = other;
	}

	virtual ~ObjectCache()
	{
		if (!PlainOldData) {
			// Make sure to call destructors of old objects.
			_Resize(0);
		}
		free(fItems);
	}

	SelfType& operator=(const SelfType& other)
	{
		if (this == &other)
			return *this;

		if (PlainOldData) {
			if (_Resize(other.fCount))
				memcpy(fItems, other.fItems, fCount * sizeof(ObjectType));
		} else {
			// Make sure to call destructors of old objects.
			// NOTE: Another option would be to use
			// ObjectType::operator=(const ObjectType& other), but then
			// we would need to be carefull which objects are already
			// initialized. Also the ObjectType requires to implement the
			// operator, while doing it this way requires only a copy
			// constructor.
			_Resize(0);
			for (uint32 i = 0; i < other.fCount; i++) {
				if (AppendObject(*other.ObjectAtFast(i)) == NULL)
					break;
			}
		}
		return *this;
	}

	bool operator==(const SelfType& other) const
	{
		if (this == &other)
			return true;

		if (fCount != other.fCount)
			return false;
		if (fCount == 0)
			return true;

		if (PlainOldData) {
			return memcmp(fItems, other.fItems,
				fCount * sizeof(ObjectType)) == 0;
		} else {
			for (uint32 i = 0; i < other.fCount; i++) {
				if (*ObjectAtFast(i) != *other.ObjectAtFast(i))
					return false;
			}
		}
		return true;
	}

	bool operator!=(const SelfType& other) const
	{
		return !(*this == other);
	}

	inline void Clear()
	{
		_Resize(0);
	}

	inline uint32 CountObjects() const
	{
		return fCount;
	}

	inline ObjectType* AppendObject()
	{
		if (_Resize(fCount + 1)) {
			ObjectType* object = fItems + fCount - 1;
			if (!PlainOldData) {
				// Initialize the new object
				new(object) ObjectType;
			}
			return object;
		}
		return NULL;
	}

	inline ObjectType* AppendObject(const ObjectType& copyFrom)
	{
		if (_Resize(fCount + 1)) {
			ObjectType* object = LastObject();
			// Initialize the new object from the original.
			if (!PlainOldData)
				new (object) ObjectType(copyFrom);
			else
				*object = copyFrom;
			return object;
		}
		return NULL;
	}

	inline bool Reserve(uint32 count)
	{
		if (count < fCount)
			return true;
		uint32 currentCount = fCount;
		if (_Resize(count))
			return false;
		fCount = currentCount;
		return true;
	}

	inline void RemoveObject()
	{
		if (fCount > 0)
			_Resize(fCount - 1);
	}

	inline ObjectType* ObjectAt(int32 index) const
	{
		if (index >= (int32)fCount)
			return NULL;
		return ObjectAtFast(index);
	}

	inline ObjectType* ObjectAtFast(int32 index) const
	{
		return fItems + index;
	}

	inline ObjectType* LastObject() const
	{
		return ObjectAt((int32)fCount - 1);
	}

private:
	inline bool _Resize(uint32 count)
	{
		if (count > fAllocatedCount) {
			uint32 allocationCount = (count + BlockSize - 1)
				/ BlockSize * BlockSize;
			ObjectType* items = reinterpret_cast<ObjectType*>(
				realloc(fItems, allocationCount * sizeof(ObjectType)));
			if (items == NULL)
				return false;
			fItems = items;

			fAllocatedCount = allocationCount;
		} else if (count < fCount) {
			if (!PlainOldData) {
				// Uninit old objects so that we can re-use them when
				// appending objects without the need to re-allocate.
				for (uint32 i = count; i < fCount; i++) {
					ObjectType* object = fItems + i;
					object->~ObjectType();
				}
			}
		}
		fCount = count;
		return true;
	}

	ObjectType*		fItems;
	uint32			fCount;
	uint32			fAllocatedCount;
};

#endif // OBJECT_CACHE_H
