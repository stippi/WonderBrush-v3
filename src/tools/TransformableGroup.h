#ifndef TRANSFORMABLE_GROUP_H
#define TRANSFORMABLE_GROUP_H

#include "HashMap.h"
#include "Transformable.h"

class Object;
class Selectable;

class TransformableGroup {
 public:
								TransformableGroup();
	virtual						~TransformableGroup();

			void				AddObject(Selectable* object);

			void				ApplyTransformation(
									const Transformable& transform);

 private:
	typedef HashMap<HashKey32<Object*>, Transformable> TransformableMap;

			TransformableMap	fInitialTransforms;
};

#endif // TRANSFORMABLE_GROUP_H
