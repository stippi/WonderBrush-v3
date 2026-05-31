#include "Test.h"

#include "Document.h"
#include "CloneContext.h"

#include <Message.h>

class CountedResource : public BaseObject {
public:
	CountedResource(int* destructorCount, int* cloneDestructorCount)
		: BaseObject()
		, fDestructorCount(destructorCount)
		, fCloneDestructorCount(cloneDestructorCount)
	{
	}

	~CountedResource() override
	{
		(*fDestructorCount)++;
	}

	BaseObject* Clone(CloneContext&) const override
	{
		return new CountedResource(fCloneDestructorCount, fCloneDestructorCount);
	}

	const char* DefaultName() const override
	{
		return "CountedResource";
	}

private:
	int* fDestructorCount;
	int* fCloneDestructorCount;
};

static void
TestDocumentCloneReleasesFreshGlobalResourceReferenceAfterInsertion()
{
	int originalDestructorCount = 0;
	int cloneDestructorCount = 0;

	{
		Document document(BRect(0, 0, 10, 10));
		CountedResource* original = new CountedResource(&originalDestructorCount,
			&cloneDestructorCount);
		ASSERT_TRUE(document.GlobalResources().AddObject(original));
		ASSERT_EQ(2, original->CountReferences());
		ASSERT_TRUE(!original->RemoveReference());
		ASSERT_EQ(1, original->CountReferences());

		CloneContext context;
		Document* clonedDocument = dynamic_cast<Document*>(document.Clone(context));
		ASSERT_TRUE(clonedDocument != 0);
		ASSERT_EQ(1, clonedDocument->CountReferences());
		ASSERT_EQ(0, cloneDestructorCount);

		ASSERT_TRUE(clonedDocument->RemoveReference());
		ASSERT_EQ(1, cloneDestructorCount);
		ASSERT_EQ(0, originalDestructorCount);
	}

	ASSERT_EQ(1, originalDestructorCount);
}

int
main()
{
	TestDocumentCloneReleasesFreshGlobalResourceReferenceAfterInsertion();
	printf("ownership-document-clone-resources: PASS\n");
	return 0;
}
