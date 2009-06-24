/*
 * Copyright (c) 2009, Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef DATA_BLOCK_H
#define DATA_BLOCK_H

#include <new>
#include <stdlib.h>

#include <SupportDefs.h>

class BString;
class RWLocker;
class Command;

template <typename DataType, uint32 BlockSize = 32>
class DataBlock {
public:
	DataBlock()
		:
		fData(NULL),
		fCurrentOffset(0),
		fSize(0),
		fAllocatedSize(0)
	{
	}

	virtual ~DataBlock()
	{
		free(fData);
	}

	inline void Clear()
	{
		_Resize(0);
		fCurrentOffset = 0;
	}

	inline uint32 Size() const
	{
		return fSize;
	}

	inline DataType* Reserve(uint32 size)
	{
//printf("%p->Reserve(%lu)\n", this, size);
		if (!_Resize(fCurrentOffset + size))
			return NULL;

		DataType* data = CurrentData();
		fCurrentOffset += size;
//printf("  success -> %lu\n", fCurrentOffset);
		return data;
	}

	inline status_t Reserve(const DataType* start, uint32 size)
	{
//printf("%p->Reserve(%p, %lu)\n", this, start, size);
		int32 offset = start - fData;
		if (offset < 0 || (uint32)offset > fSize) {
//printf("  INVALID!\n");
			// "start" is not within our allocated block of memory!
			return B_BAD_VALUE;
		}
		if (!_Resize(offset + size))
			return B_NO_MEMORY;

		fCurrentOffset = offset + size;
//printf("  success -> %lu\n", fCurrentOffset);
		return B_OK;
	}

	inline DataType* CurrentData() const
	{
		return DataAtFast(fCurrentOffset);
	}

	inline DataType* DataAt(uint32 index) const
	{
		if (index > fSize)
			return NULL;
		return DataAtFast(index);
	}

	inline DataType* DataAtFast(uint32 index) const
	{
		return fData + index;
	}

private:
	inline bool _Resize(uint32 size)
	{
		if (size > fAllocatedSize) {
			uint32 allocationSize = (size + BlockSize - 1)
				/ BlockSize * BlockSize;
			DataType* items = reinterpret_cast<DataType*>(
				realloc(fData, allocationSize * sizeof(DataType)));
			if (items == NULL)
				return false;
			fData = items;
			fAllocatedSize = allocationSize;
		}
		fSize = size;
		return true;
	}

	DataType*		fData;
	uint32			fCurrentOffset;
	uint32			fSize;
	uint32			fAllocatedSize;
};

#endif // DATA_BLOCK_H
