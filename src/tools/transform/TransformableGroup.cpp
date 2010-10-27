#include "TransformableGroup.h"

#include "Object.h"
#include "Selectable.h"

// constructor
TransformableGroup::TransformableGroup()
	: fInitialTransforms()
{
}

// destructor
TransformableGroup::~TransformableGroup()
{
}

// AddObject
void
TransformableGroup::AddObject(Selectable* selectable)
{
	Object* object = dynamic_cast<Object*>(selectable);

	if (!object || !object->IsRegularTransformable())
		return;


	if (fInitialTransforms.Put(object, object->LocalTransformation()) == B_OK)
		object->AddReference();
}

// ApplyTransformation
void
TransformableGroup::ApplyTransformation(const Transformable& transform)
{
	TransformableMap::Iterator iterator = fInitialTransforms.GetIterator();
	while (iterator.HasNext()) {
		const TransformableMap::Entry& entry = iterator.Next();
		Transformable t = entry.value;
		t.Multiply(transform);
		entry.key.value->SetTransformable(t);
	}
}

