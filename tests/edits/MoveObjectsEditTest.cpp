#include "Test.h"

#include "Layer.h"
#include "MoveObjectsEdit.h"

#include <Message.h>
#include <String.h>

// Focused edit seam: provide the BaseObject/Notifier/UndoableEdit/Selection
// methods needed by Object/Layer/MoveObjectsEdit without pulling in the full UI.
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

UndoableEdit::UndoableEdit()
	: Referenceable()
	, fTimeStamp(0)
{
}

UndoableEdit::~UndoableEdit() {}
status_t UndoableEdit::InitCheck() { return B_OK; }
status_t UndoableEdit::Perform(EditContext&) { return B_OK; }
status_t UndoableEdit::Undo(EditContext&) { return B_OK; }
status_t UndoableEdit::Redo(EditContext& context) { return Perform(context); }
void UndoableEdit::GetName(BString&) {}
bool UndoableEdit::UndoesPrevious(const UndoableEdit*) { return false; }
bool UndoableEdit::CombineWithNext(const UndoableEdit*) { return false; }
bool UndoableEdit::CombineWithPrevious(const UndoableEdit*) { return false; }

Selection::Controller::Controller() {}
Selection::Controller::~Controller() {}
bool Selection::Select(const Selectable&, const Controller*, bool) { return true; }
void Selection::DeselectAll(const Controller*) {}

Selectable::Selectable()
	: Reference<BaseObject>()
{
}

Selectable::Selectable(BaseObject* object)
	: Reference<BaseObject>(object)
{
}

Selectable::Selectable(const Selectable& other)
	: Reference<BaseObject>(other)
{
}

Selectable::~Selectable() {}

class CountedObject : public Object {
public:
	CountedObject(int* destructorCount)
		: Object()
		, fDestructorCount(destructorCount)
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
		return new CountedObject(fDestructorCount);
	}

	const char* DefaultName() const override
	{
		return "CountedObject";
	}

private:
	int* fDestructorCount;
};

static void
TestNestedSelectionNoOpMoveIsRejected()
{
	int childDestructorCount = 0;

	{
		Layer root(BRect(0, 0, 100, 100));
		Layer* childLayer = new Layer(BRect(0, 0, 50, 50));
		CountedObject* child = new CountedObject(&childDestructorCount);

		ASSERT_TRUE(root.AddObject(childLayer));
		ASSERT_TRUE(!childLayer->RemoveReference());
		ASSERT_TRUE(childLayer->AddObject(child));
		ASSERT_TRUE(!child->RemoveReference());

		Object** objects = new Object*[2];
		objects[0] = childLayer;
		objects[1] = child;

		MoveObjectsEdit edit(objects, 2, &root, 0, 0);
		ASSERT_EQ(B_BAD_VALUE, edit.InitCheck());
	}

	ASSERT_EQ(1, childDestructorCount);
}

int
main()
{
	TestNestedSelectionNoOpMoveIsRejected();
	printf("edits-move-objects: PASS\n");
	return 0;
}
