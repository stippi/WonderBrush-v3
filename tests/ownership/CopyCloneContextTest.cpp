#include "Test.h"

#include "CopyCloneContext.h"

#include <Message.h>

class CountedBaseObject : public BaseObject {
public:
	CountedBaseObject(int* destructorCount, int* cloneDestructorCount)
		: BaseObject()
		, fDestructorCount(destructorCount)
		, fCloneDestructorCount(cloneDestructorCount)
	{
	}

	~CountedBaseObject() override
	{
		(*fDestructorCount)++;
	}

	BaseObject* Clone(CloneContext&) const override
	{
		return new CountedBaseObject(fCloneDestructorCount, fCloneDestructorCount);
	}

	const char* DefaultName() const override
	{
		return "CountedBaseObject";
	}

private:
	int* fDestructorCount;
	int* fCloneDestructorCount;
};

static void
TestCopyCloneContextAdoptsFreshCloneReference()
{
	int originalDestructorCount = 0;
	int cloneDestructorCount = 0;

	CountedBaseObject* original = new CountedBaseObject(&originalDestructorCount,
		&cloneDestructorCount);

	{
		CopyCloneContext context;
		BaseObjectRef clone = context.Clone(original);
		ASSERT_TRUE(clone.Get() != 0);
		ASSERT_EQ(1, clone->CountReferences());
	}

	ASSERT_EQ(1, cloneDestructorCount);
	ASSERT_EQ(0, originalDestructorCount);
	ASSERT_TRUE(original->RemoveReference());
	ASSERT_EQ(1, originalDestructorCount);
}

int
main()
{
	TestCopyCloneContextAdoptsFreshCloneReference();
	printf("ownership-copy-clone-context: PASS\n");
	return 0;
}
