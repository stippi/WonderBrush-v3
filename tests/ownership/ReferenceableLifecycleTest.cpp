#include "Test.h"

#include "Referenceable.h"

class CountedReferenceable : public Referenceable {
public:
	CountedReferenceable(int* destructorCount)
		: Referenceable(true)
		, fDestructorCount(destructorCount)
	{
	}

	~CountedReferenceable() override
	{
		(*fDestructorCount)++;
	}

private:
	int* fDestructorCount;
};

static void
TestInitialReferenceAndRemovalDeletesObject()
{
	int destructorCount = 0;
	CountedReferenceable* object = new CountedReferenceable(&destructorCount);

	ASSERT_EQ(1, object->CountReferences());
	ASSERT_TRUE(object->RemoveReference());
	ASSERT_EQ(1, destructorCount);
}

static void
TestReferenceAdoptsExistingReferenceWithoutIncrementing()
{
	int destructorCount = 0;
	CountedReferenceable* object = new CountedReferenceable(&destructorCount);

	{
		Reference<CountedReferenceable> reference(object, true);
		ASSERT_EQ(1, reference->CountReferences());
	}

	ASSERT_EQ(1, destructorCount);
}

static void
TestReferenceAddsReferenceWhenNotAdopting()
{
	int destructorCount = 0;
	CountedReferenceable* object = new CountedReferenceable(&destructorCount);

	{
		Reference<CountedReferenceable> reference(object);
		ASSERT_EQ(2, reference->CountReferences());
	}

	ASSERT_EQ(1, object->CountReferences());
	ASSERT_EQ(0, destructorCount);
	ASSERT_TRUE(object->RemoveReference());
	ASSERT_EQ(1, destructorCount);
}

int
main()
{
	TestInitialReferenceAndRemovalDeletesObject();
	TestReferenceAdoptsExistingReferenceWithoutIncrementing();
	TestReferenceAddsReferenceWhenNotAdopting();
	printf("ownership-referenceable: PASS\n");
	return 0;
}
