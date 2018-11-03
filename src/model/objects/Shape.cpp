/*
 * Copyright 2007-2018, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "Shape.h"

#include "PathInstance.h"
#include "ShapeSnapshot.h"

class PathListener : public Path::Listener {
public:
	PathListener(Shape* shape)
		: fShape(shape)
	{
	}

	virtual ~PathListener()
	{
	}

	virtual	void PointAdded(const Path* path, int32 index)
	{
		fShape->NotifyAndUpdate();
	}
	
	virtual	void PointRemoved(const Path* path, int32 index)
	{
		fShape->NotifyAndUpdate();
	}
	
	virtual	void PointChanged(const Path* path, int32 index)
	{
		fShape->NotifyAndUpdate();
	}
	
	virtual	void PathChanged(const Path* path)
	{
		fShape->NotifyAndUpdate();
	}
	
	virtual	void PathClosedChanged(const Path* path)
	{
		fShape->NotifyAndUpdate();
	}
	
	virtual	void PathReversed(const Path* path)
	{
		fShape->NotifyAndUpdate();
	}

private:
	Shape*	fShape;
};


Shape::Listener::Listener() {}
Shape::Listener::~Listener() {}


// #pragma mark -

// constructor
Shape::Shape()
	: Styleable()
	, fPathListener(new(std::nothrow) PathListener(this))
	, fFillMode(FILL_MODE_NON_ZERO)
	, fListeners()
{
	InitBounds();
}

// constructor
Shape::Shape(const PathRef& path, const rgb_color& color)
	: Styleable(color)
	, fPathListener(new(std::nothrow) PathListener(this))
	, fFillMode(FILL_MODE_NON_ZERO)
	, fListeners()
{
	AddPath(path);
	InitBounds();
}

// constructor
Shape::Shape(const Shape& other, CloneContext& context)
	: Styleable(other, context)
	, fPathListener(new(std::nothrow) PathListener(this))
	, fFillMode(other.fFillMode)
	, fListeners()
{
	int32 count = other.Paths().CountItems();
	for (int32 i = 0; i < count; i++) {
		const PathRef& path = other.Paths().ItemAtFast(i)->Path();
		PathRef clonedPath;
		context.Clone(path.Get(), clonedPath);
		if (clonedPath.Get() != NULL)
			AddPath(clonedPath);
	}
	InitBounds();
}

// destructor
Shape::~Shape()
{
	int32 count = fPaths.CountItems();
	for (int32 i = 0; i < count; i++) {
		const PathRef& path = fPaths.ItemAtFast(i)->Path();
		path->RemoveListener(fPathListener);
	}
	delete fPathListener;
}

// #pragma mark -

// Clone
BaseObject*
Shape::Clone(CloneContext& context) const
{
	return new(std::nothrow) Shape(*this, context);
}

// DefaultName
const char*
Shape::DefaultName() const
{
	return "Shape";
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Shape::Snapshot() const
{
	return new ShapeSnapshot(this);
}

// Assets
AssetList
Shape::Assets() const
{
	AssetList list = Styleable::Assets();
	for (int32 i = 0; i < fPaths.CountItems(); i++) {
		const PathRef& ref = fPaths.ItemAtFast(i)->Path();
		list.Add(BaseObjectRef(ref.Get()));
	}
	return list;
}

// AddProperties
void
Shape::AddProperties(PropertyObject* object, uint32 flags) const
{
	Styleable::AddProperties(object, flags);
	// TODO: fFillMode
}

// SetToPropertyObject
bool
Shape::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	// TODO: fFillMode
	return Styleable::SetToPropertyObject(object, flags);
}

// HitTest
bool
Shape::HitTest(const BPoint& canvasPoint)
{
	PathStorage path;
	GetPath(path);
	RenderEngine engine(Transformation());
	// TODO: fFillMode
	return engine.HitTest(path, canvasPoint);
}

// #pragma mark -

// SetFillMode
void
Shape::SetFillMode(uint32 fillMode)
{
	if (fFillMode != fillMode) {
		fFillMode = fillMode;
		NotifyAndUpdate();
	}
}

// AddPath
PathInstance*
Shape::AddPath(const PathRef& path)
{
	return AddPath(path, fPaths.CountItems());
}

// AddPath
PathInstance*
Shape::AddPath(const PathRef& path, int32 index)
{
	if (index < 0 || index > fPaths.CountItems())
		return NULL;
	if (path.Get() == NULL)
		return NULL;
	PathInstance* pathInstance = new(std::nothrow) PathInstance(path.Get(),
		this);
	PathInstanceRef ref(pathInstance, true);
	if (pathInstance == NULL || !fPaths.Add(ref, index))
		return NULL;
	
	if (fPathListener != NULL)
		path->AddListener(fPathListener);

	NotifyAndUpdate();
	_NotifyPathAdded(pathInstance, index);
	return pathInstance;
}

// RemovePath
bool
Shape::RemovePath(const PathRef& path)
{
	for (int32 i = fPaths.CountItems() - 1; i >= 0; i--) {
		PathInstanceRef pathInstance = fPaths.ItemAtFast(i);
		if (pathInstance->Path() == path.Get()) {
			pathInstance->Path()->RemoveListener(fPathListener);
			fPaths.Remove(i);
			_NotifyPathRemoved(pathInstance, i);
			return true;
		}
	}
	return false;
}

// ContainsPath
bool
Shape::ContainsPath(const PathRef& path) const
{
	for (int32 i = fPaths.CountItems() - 1; i >= 0; i--) {
		PathInstanceRef pathInstance = fPaths.ItemAtFast(i);
		if (pathInstance->Path() == path.Get())
			return true;
	}
	return false;
}

// Paths
const PathList&
Shape::Paths() const
{
	return fPaths;
}

// Bounds
BRect
Shape::Bounds()
{
	BRect bounds(LONG_MAX, LONG_MAX, -LONG_MAX, -LONG_MAX);
	for (int32 i = fPaths.CountItems() - 1; i >= 0; i--) {
		const PathRef& path = fPaths.ItemAtFast(i)->Path();
		bounds = bounds | path->Bounds();
	}
	if (bounds.IsValid())
		Style()->ExtendBounds(bounds);
	return bounds;
}

// GetPath
void
Shape::GetPath(PathStorage& pathStorage) const
{
	for (int32 i = fPaths.CountItems() - 1; i >= 0; i--) {
		const PathRef& path = fPaths.ItemAtFast(i)->Path();
		path->GetAGGPathStorage(pathStorage);
	}
}

// AddShapeListener
void
Shape::AddShapeListener(Listener* listener)
{
	fListeners.Add(listener);
}

// RemoveShapeListener
void
Shape::RemoveShapeListener(Listener* listener)
{
	fListeners.Remove(listener);
}

// _NotifyPathAdded
void
Shape::_NotifyPathAdded(const PathInstanceRef& path, int32 index) const
{
	List<Listener*, false> listeners(fListeners);
	int32 count = listeners.CountItems();
	for (int32 i = 0; i < count; i++) {
		listeners.ItemAtFast(i)->PathAdded(this, path, index);
	}
}

// _NotifyPathRemoved
void
Shape::_NotifyPathRemoved(const PathInstanceRef& path, int32 index) const
{
	List<Listener*, false> listeners(fListeners);
	int32 count = listeners.CountItems();
	for (int32 i = 0; i < count; i++) {
		listeners.ItemAtFast(i)->PathRemoved(this, path, index);
	}
}
