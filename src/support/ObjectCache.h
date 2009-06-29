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
public:
	ObjectCache()
		:
		fItems(NULL),
		fCount(0),
		fAllocatedCount(0)
	{
	}

	virtual ~ObjectCache()
	{
		free(fItems);
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
			ObjectType* object = LastObject();
			if (!PlainOldData) {
				// Initialize the new object
				new (object) ObjectType;
			}
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

	inline ObjectType* ObjectAt(uint32 index) const
	{
		if (index > fCount)
			return NULL;
		return ObjectAtFast(index);
	}

	inline ObjectType* ObjectAtFast(uint32 index) const
	{
		return fItems + index;
	}

	inline ObjectType* LastObject() const
	{
		return ObjectAtFast(fCount - 1);
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
