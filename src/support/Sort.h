// Sort.h
//
// A recursive and an iterative merge sort implementation.
// Both need a second buffer for their work. The basic versions
// take such a buffer as an argument and return either this buffer or
// the data buffer depending on where the sorted data are stored.
// For convenience there are wrapper versions that allocate and free
// the buffer on their own and ensure that afterwards the sorted data
// are stored in the original data buffer.
// The versions that don't take a `greater' comparator argument use the
// standard `>'.
// The recursive versions:
// * merge_sort(DATA* data, DATA* buffer, int32 count,
//				const DataGreater& greater)
// * merge_sort(DATA* data, DATA* buffer, int32 count)
// * merge_sort(DATA* data, int32 count, const DataGreater& greater)
// * merge_sort(DATA* data, int32 count)
// The iterative ones (limited to 2^(kIterativeMergeSortStackSize - 1) items):
// * merge_sort_iterative(DATA* data, DATA* buffer, int32 count,
//						  const DataGreater& greater)
// * merge_sort_iterative(DATA* data, DATA* buffer, int32 count)
// * merge_sort_iterative(DATA* data, int32 count, const DataGreater& greater)
// * merge_sort_iterative(DATA* data, int32 count)

#ifndef SORT_H
#define SORT_H

#include <SupportDefs.h>

static const int kIterativeMergeSortStackSize = 32;

template<typename C>
class StdGreater {
 public:
	inline	bool	operator()(C a, C b) const
	{
		return (a > b);
	}
};


// recursive versions

template<typename DATA, typename DataGreater >
DATA*
merge_sort(DATA* data, DATA* buffer, int32 count, const DataGreater& greater)
{
	DATA* dest = data;
	if (count > 1) {
		// split
		int32 count1 = count / 2;
		int32 count2 = count - count1;
		DATA* sorted1 = merge_sort(data, buffer, count1);
		DATA* sorted2 = merge_sort(data + count1, buffer + count1, count2);
		// merge
		if (sorted1 == data)
			dest = buffer;
		DATA* d = dest;
		for (; count1 > 0 && count2 > 0; d++) {
			if (greater(*sorted1, *sorted2)) {
				*d = *sorted2;
				sorted2++;
				count2--;
			} else {
				*d = *sorted1;
				sorted1++;
				count1--;
			}
		}
		// insert left data from the first part
		for (; count1 > 0; d++, sorted1++, count1--)
			*d = *sorted1;
		// insert left data from the second part
		for (; count2 > 0; d++, sorted2++, count2--)
			*d = *sorted2;
	}
	return dest;
}

template<typename DATA>
inline
DATA*
merge_sort(DATA* data, DATA* buffer, int32 count)
{
	return merge_sort(data, buffer, count, StdGreater<DATA>());
}

template<typename DATA, typename DataGreater >
inline
void
merge_sort(DATA* data, int32 count, const DataGreater& greater)
{
	if (count > 1) {
		DATA* buffer = new DATA[count];
		DATA* result = merge_sort(data, buffer, count, greater);
		if (result == buffer)
			memcpy(data, buffer, count * sizeof(DATA));
		delete[] buffer;
	}
}

template<typename DATA>
inline
void
merge_sort(DATA* data, int32 count)
{
	merge_sort(data, count, StdGreater<DATA>());
}


// iterative versions

template<typename DATA, typename DataGreater>
DATA*
merge_sort_iterative(DATA* data, DATA* buffer, int32 count,
					 const DataGreater& greater)
{
	struct work_data {
		DATA* 	data;
		DATA*	buffer;
		int32	count;
		int32	depth;
	};

	struct result_data {
		DATA* 	result;
		DATA* 	buffer;
		int32	count;
		int32	depth;
	};

	// work and result stack
	work_data	work[kIterativeMergeSortStackSize];
	result_data	results[kIterativeMergeSortStackSize];
	int32		workDepth = -1;
	int32		resultDepth = -1;
	DATA* result = data;
	// push (data, buffer, count) on work stack
	if (count > 1) {
		workDepth++;
		work[workDepth].data = data;
		work[workDepth].buffer = buffer;
		work[workDepth].count = count;
		work[workDepth].depth = 0;
	}
	while (workDepth >= 0) {
		// pop work
		data = work[workDepth].data;
		buffer = work[workDepth].buffer;
		count = work[workDepth].count;
		int32 depth = work[workDepth].depth;
		workDepth--;
		while (count > 1) {
			// split and push second on work stack
			depth++;
			workDepth++;
			work[workDepth].count = count / 2;
			count -= work[workDepth].count;
			work[workDepth].data = data + count;
			work[workDepth].buffer = buffer + count;
			work[workDepth].depth = depth;
		}
		// set result
		result = data;
		while (resultDepth >= 0 && results[resultDepth].depth == depth) {
			// pop result stack
			// merge and set new result
			DATA* sorted1 = results[resultDepth].result;
			DATA* sorted2 = result;
			int32 count1 = results[resultDepth].count;
			int32 count2 = count;
			count += count1;
			result = results[resultDepth].buffer;
			buffer = sorted1;
			depth--;
			resultDepth--;
			DATA* d = result;
			for (; count1 > 0 && count2 > 0; d++) {
				if (greater(*sorted1, *sorted2)) {
					*d = *sorted2;
					sorted2++;
					count2--;
				} else {
					*d = *sorted1;
					sorted1++;
					count1--;
				}
			}
			// insert left data from the first part
			for (; count1 > 0; d++, sorted1++, count1--)
				*d = *sorted1;
			// insert left data from the second part
			for (; count2 > 0; d++, sorted2++, count2--)
				*d = *sorted2;
		}
		// push result onto the result stack
		resultDepth++;
		results[resultDepth].result = result;
		results[resultDepth].buffer = buffer;
		results[resultDepth].count = count;
		results[resultDepth].depth = depth;
	}
	return result;
}

template<typename DATA>
inline
DATA*
merge_sort_iterative(DATA* data, DATA* buffer, int32 count)
{
	return merge_sort_iterative(data, buffer, count, StdGreater<DATA>());
}

template<typename DATA, typename DataGreater >
inline
void
merge_sort_iterative(DATA* data, int32 count, const DataGreater& greater)
{
	if (count > 1) {
		DATA* buffer = new DATA[count];
		DATA* result = merge_sort_iterative(data, buffer, count, greater);
		if (result == buffer)
			memcpy(data, buffer, count * sizeof(DATA));
		delete[] buffer;
	}
}

template<typename DATA>
inline
void
merge_sort_iterative(DATA* data, int32 count)
{
	merge_sort_iterative(data, count, StdGreater<DATA>());
}


#endif	// SORT_H

