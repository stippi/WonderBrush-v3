/*
 * Copyright (c) 2009, Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef OBJECT_CACHE_H
#define OBJECT_CACHE_H

#include <new>
#include <stdlib.h>

#include <SupportDefs.h>

template <typename ObjectType, uint32 BlockSize = 8>
class ObjectCache {
public:
	ObjectCache()
		:
		fItems(NULL),
		fNullObject(),
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
		// TODO: The object will not really be initialized if
		// it wasn't just allocated! This only works for how this class is
		// currently used!
		if (_Resize(fCount + 1)) {
			ObjectType* object = LastObject();
//			*object = fNullObject;
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

			// initialize the new objects
			for (uint32 i = fAllocatedCount; i < allocationCount; i++) {
				ObjectType* object = fItems + i;
				new (object) ObjectType;
			}

			fAllocatedCount = allocationCount;
		}
		fCount = count;
		return true;
	}

	ObjectType*		fItems;
	ObjectType		fNullObject;
	uint32			fCount;
	uint32			fAllocatedCount;
};

#endif // OBJECT_CACHE_H
