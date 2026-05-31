#include "Test.h"

#include "Layer.h"
#include "NotifyingList.h"

#include <Message.h>
#include <type_traits>
#include <utility>

class CountedObject : public Object {
public:
	CountedObject(int* destructorCount, int* cloneDestructorCount)
		: Object()
		, fDestructorCount(destructorCount)
		, fCloneDestructorCount(cloneDestructorCount)
	{
	}

	~CountedObject() override
	{
		(*fDestructorCount)++;
	}

	ObjectSnapshot* Snapshot() const override
	{
		return 0;
	}

	BaseObject* Clone(CloneContext&) const override
	{
		return new CountedObject(fCloneDestructorCount, fCloneDestructorCount);
	}

	const char* DefaultName() const override
	{
		return "CountedObject";
	}

private:
	int* fDestructorCount;
	int* fCloneDestructorCount;
};

static void
TestCloneObjectsReleasesFreshCloneReferenceAfterInsertion()
{
	int originalDestructorCount = 0;
	int cloneDestructorCount = 0;

	{
		Layer source(BRect(0, 0, 10, 10));
		CountedObject* original = new CountedObject(&originalDestructorCount,
			&cloneDestructorCount);
		ASSERT_TRUE(source.AddObject(original));
		ASSERT_EQ(2, original->CountReferences());
		ASSERT_TRUE(!original->RemoveReference());
		ASSERT_EQ(1, original->CountReferences());

		CloneContext context;
		Layer* clonedLayer = dynamic_cast<Layer*>(source.Clone(context));
		ASSERT_TRUE(clonedLayer != 0);
		ASSERT_EQ(1, clonedLayer->CountReferences());
		ASSERT_EQ(0, cloneDestructorCount);

		ASSERT_TRUE(clonedLayer->RemoveReference());
		ASSERT_EQ(1, cloneDestructorCount);
		ASSERT_EQ(0, originalDestructorCount);
	}

	ASSERT_EQ(1, originalDestructorCount);
}

static void
TestLayerRejectsDuplicateObjectInsertion()
{
	int destructorCount = 0;
	int cloneDestructorCount = 0;

	{
		Layer layer(BRect(0, 0, 10, 10));
		CountedObject* object = new CountedObject(&destructorCount,
			&cloneDestructorCount);

		ASSERT_TRUE(layer.AddObject(object));
		ASSERT_EQ(1, layer.CountObjects());
		ASSERT_TRUE(!object->RemoveReference());
		ASSERT_EQ(1, object->CountReferences());

		ASSERT_TRUE(!layer.AddObject(object));
		ASSERT_EQ(1, layer.CountObjects());
		ASSERT_EQ(1, object->CountReferences());
	}

	ASSERT_EQ(1, destructorCount);
}

static void
TestRemoveObjectIndexAPIsDoNotReturnReleasedPointers()
{
	static_assert(std::is_same<decltype(std::declval<Layer&>().RemoveObject(0)),
		Reference<Object> >::value,
		"Layer::RemoveObject(index) must return a reference that keeps the removed object alive");
	static_assert(std::is_same<decltype(std::declval<NotifyingList<CountedObject>&>().RemoveObject(0)),
		Reference<CountedObject> >::value,
		"NotifyingList::RemoveObject(index) must return a reference that keeps the removed object alive");

	int layerDestructorCount = 0;
	int listDestructorCount = 0;
	int cloneDestructorCount = 0;
	{
		Layer layer(BRect(0, 0, 10, 10));
		CountedObject* object = new CountedObject(&layerDestructorCount,
			&cloneDestructorCount);
		ASSERT_TRUE(layer.AddObject(object));
		ASSERT_TRUE(!object->RemoveReference());
		Reference<Object> removed = layer.RemoveObject(0);
		ASSERT_TRUE(removed.Get() == object);
		ASSERT_EQ(0, layerDestructorCount);
		removed.Unset();
		ASSERT_EQ(1, layerDestructorCount);
	}
	ASSERT_EQ(1, layerDestructorCount);

	{
		NotifyingList<CountedObject> list;
		CountedObject* object = new CountedObject(&listDestructorCount,
			&cloneDestructorCount);
		ASSERT_TRUE(list.AddObject(object));
		ASSERT_TRUE(!object->RemoveReference());
		Reference<CountedObject> removed = list.RemoveObject(0);
		ASSERT_TRUE(removed.Get() == object);
		ASSERT_EQ(0, listDestructorCount);
		removed.Unset();
		ASSERT_EQ(1, listDestructorCount);
	}
	ASSERT_EQ(1, listDestructorCount);
}

int
main()
{
	TestCloneObjectsReleasesFreshCloneReferenceAfterInsertion();
	TestLayerRejectsDuplicateObjectInsertion();
	TestRemoveObjectIndexAPIsDoNotReturnReleasedPointers();
	printf("ownership-layer-clone-objects: PASS\n");
	return 0;
}
