/*
 * Copyright 2005-2007 Ingo Weinhold <ingo_weinhold@gmx.de>
 */

#include <typeinfo>

#include <dlfcn.h>
#include <map>
#include <stdlib.h>

#include <Autolock.h>
#include <image.h>
#include <Locker.h>
#include <String.h>

#include "Debug.h"
#include "Referenceable.h"
#include "StackTrace.h"

#define DEBUG_REFERENCES 0

// constructor
Referenceable::Referenceable(bool deleteWhenUnreferenced)
	: fReferenceCount(1),
	  fDeleteWhenUnreferenced(deleteWhenUnreferenced)
{
}

// destructor
Referenceable::~Referenceable()
{
}

// AddReference
void
Referenceable::AddReference()
{
#if DEBUG_REFERENCES
	int32 count = atomic_add(&fReferenceCount, 1);
	BAutolock _(get_stack_trace_locker());
	printf("[%ld] %p->AddReference(): %ld (%s)\n", find_thread(NULL), this,
		count + 1, typeid(*this).name());
	print_stack_trace();
#else
	atomic_add(&fReferenceCount, 1);
#endif
}

// RemoveReference
bool
Referenceable::RemoveReference()
{
#if DEBUG_REFERENCES
	int32 oldCount = atomic_add(&fReferenceCount, -1);
	{
		BAutolock _(get_stack_trace_locker());
		printf("[%ld] %p->RemoveReference(): %ld (%s)\n", find_thread(NULL),
			this, oldCount - 1, typeid(*this).name());
		print_stack_trace();
	}
	bool unreferenced = oldCount == 1;
#else
	bool unreferenced = (atomic_add(&fReferenceCount, -1) == 1);
#endif
	if (fDeleteWhenUnreferenced && unreferenced)
		delete this;
	return unreferenced;
}

// CountReferences
int32
Referenceable::CountReferences() const
{
	return fReferenceCount;
}

