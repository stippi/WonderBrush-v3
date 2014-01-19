/*
 * Copyright 2005-2007 Ingo Weinhold <ingo_weinhold@gmx.de>
 */

#ifndef REFERENCEABLE_H
#define REFERENCEABLE_H

#include <SupportDefs.h>

#include "ObjectTracker.h"

// Referenceable
class Referenceable ONLY_OBJECT_TRACKABLE_BASE_CLASS {
public:
								Referenceable(
									bool deleteWhenUnreferenced = true);
	virtual						~Referenceable();

			void				AddReference();
			bool				RemoveReference();	// returns true after last

			int32				CountReferences() const;

protected:
			int32				fReferenceCount;
			bool				fDeleteWhenUnreferenced;
};

// Reference
template<typename Type>
class Reference {
public:
	Reference()
		: fObject(NULL)
	{
	}

	Reference(Type* object, bool alreadyHasReference = false)
		: fObject(NULL)
	{
		SetTo(object, alreadyHasReference);
	}

	Reference(const Reference<Type>& other)
		: fObject(NULL)
	{
		SetTo(other.fObject);
	}

	virtual ~Reference()
	{
		Unset();
	}

	bool SetTo(Type* object, bool alreadyHasReference = false)
	{
		if (object == fObject) {
			if (alreadyHasReference)
				fObject->RemoveReference();
			return false;
		}
		Unset();
		fObject = object;
		if (fObject && !alreadyHasReference)
			fObject->AddReference();
		return true;
	}

	void Unset()
	{
		if (fObject) {
			fObject->RemoveReference();
			fObject = NULL;
		}
	}

	Type* Get() const
	{
		return fObject;
	}

	Type* Detach()
	{
		Type* object = fObject;
		fObject = NULL;
		return object;
	}

	Type& operator*() const
	{
		return *fObject;
	}

	Type* operator->() const
	{
		return fObject;
	}

	Reference& operator=(const Reference<Type>& other)
	{
		if (&other != this)
			SetTo(other.fObject);
		return *this;
	}

	bool operator==(const Reference<Type>& other) const
	{
		return (fObject == other.fObject);
	}

	bool operator!=(const Reference<Type>& other) const
	{
		return (fObject != other.fObject);
	}

private:
	Type*	fObject;
};

#endif	// REFERENCEABLE_H
