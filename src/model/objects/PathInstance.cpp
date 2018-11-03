/*
 * Copyright 2007-2018, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "PathInstance.h"

// constructor
PathInstance::PathInstance(::Path* path)
	: BaseObject()
	, fPath(path)
	, fShape(NULL)
{
}

// constructor
PathInstance::PathInstance(::Path* path, ::Shape* shape)
	: BaseObject()
	, fPath(path)
	, fShape(shape)
{
}

// constructor
PathInstance::PathInstance(const PathInstance& other, CloneContext& context)
	: BaseObject(other)
	, fPath()
	, fShape(other.fShape)
{
	context.Clone(other.fPath.Get(), fPath);
}

// destructor
PathInstance::~PathInstance()
{
}

// #pragma mark -

// Clone
BaseObject*
PathInstance::Clone(CloneContext& context) const
{
	return new(std::nothrow) PathInstance(*this, context);
}

// DefaultName
const char*
PathInstance::DefaultName() const
{
	if (fPath.Get() != NULL)
		return fPath->DefaultName();
	return "PathInstance";
}

// GetIcon
bool
PathInstance::GetIcon(const BBitmap* bitmap) const
{
	if (fPath.Get() != NULL)
		return fPath->GetIcon(bitmap);

	return false;
}

// Transformation
Transformable
PathInstance::Transformation() const
{
	if (fShape != NULL)
		return fShape->Transformation();
	return ::Transformable();
}

