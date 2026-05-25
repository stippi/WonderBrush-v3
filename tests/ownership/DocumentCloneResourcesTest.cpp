#include "Test.h"

#include "Document.h"
#include "CloneContext.h"

#include <Message.h>

// Focused ownership seam for Document::Clone() without the full UI/property
// stack. Same minimal BaseObject/Notifier implementation pattern as the other
// ownership tests.
Notifier::Notifier()
	: fListeners(2)
	, fSuspended(0)
	, fPendingNotifications(false)
{
}

Notifier::~Notifier() {}
bool Notifier::AddListener(Listener*) { return false; }
bool Notifier::RemoveListener(Listener*) { return false; }
int32 Notifier::CountListeners() const { return 0; }
Listener* Notifier::ListenerAtFast(int32) const { return 0; }
void Notifier::Notify() {}
void Notifier::SuspendNotifications(bool) {}
void Notifier::NotifyListeners() {}

BaseObject::BaseObject()
	: Notifier()
	, Referenceable()
	, fName()
{
}

BaseObject::BaseObject(const BaseObject&)
	: Notifier()
	, Referenceable()
	, fName()
{
}

BaseObject::BaseObject(const BMessage*)
	: Notifier()
	, Referenceable()
	, fName()
{
}

BaseObject::~BaseObject() {}
BaseObject* BaseObject::Clone() const { CloneContext context; return Clone(context); }
status_t BaseObject::Unarchive(const BMessage*) { return B_OK; }
status_t BaseObject::Archive(BMessage*, bool) const { return B_OK; }
PropertyObject* BaseObject::MakePropertyObject() const { return 0; }
void BaseObject::AddProperties(PropertyObject*, uint32) const {}
bool BaseObject::SetToPropertyObject(const PropertyObject*, uint32) { return false; }
void BaseObject::SetName(const char*) {}
const char* BaseObject::Name() const { return DefaultName(); }
const BString& BaseObject::GivenName() const { return fName; }
bool BaseObject::GetIcon(const BBitmap*) const { return false; }

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
