#include "Test.h"

#include "CopyCloneContext.h"

#include <Message.h>

// Minimal out-of-line implementations for the BaseObject/Notifier methods that
// are needed by this focused ownership test. This keeps the test independent of
// the GUI/property implementation and documents the seam explicitly.
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
