#include "Test.h"

#include "Layer.h"

#include <Message.h>

// Focused ownership seam: provide the BaseObject/Notifier methods needed by
// Object/Layer without pulling in the full UI/property stack.
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

int
main()
{
	TestCloneObjectsReleasesFreshCloneReferenceAfterInsertion();
	printf("ownership-layer-clone-objects: PASS\n");
	return 0;
}
