#include "Test.h"

#include "Layer.h"
#include "AddObjectsEdit.h"
#include "MoveObjectsEdit.h"

#include <Message.h>
#include <String.h>

// Edit-specific test seams that are not provided by the shared model stubs.
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

class EditContext {
};

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
TestAddObjectsEditRollsBackPartialInsertionFailure()
{
	int destructorCount = 0;

	{
		Layer layer(BRect(0, 0, 100, 100));
		CountedObject* object = new CountedObject(&destructorCount);

		Object** objects = new Object*[2];
		objects[0] = object;
		objects[1] = object;

		{
			AddObjectsEdit edit(objects, 2, &layer, 0, 0);
			ASSERT_TRUE(!object->RemoveReference());

			EditContext context;
			ASSERT_EQ(B_NO_MEMORY, edit.Perform(context));
			ASSERT_EQ(0, layer.CountObjects());
		}

		ASSERT_EQ(1, destructorCount);
	}
}

static void
TestMoveObjectsEditRestoresSourceOnPartialInsertionFailure()
{
	int destructorCount = 0;

	{
		Layer source(BRect(0, 0, 100, 100));
		Layer target(BRect(0, 0, 100, 100));
		CountedObject* object = new CountedObject(&destructorCount);
		ASSERT_TRUE(source.AddObject(object));
		ASSERT_TRUE(!object->RemoveReference());

		Object** objects = new Object*[2];
		objects[0] = object;
		objects[1] = object;

		{
			MoveObjectsEdit edit(objects, 2, &target, 0, 0);
			EditContext context;
			ASSERT_EQ(B_NO_MEMORY, edit.Perform(context));
			ASSERT_EQ(1, source.CountObjects());
			ASSERT_TRUE(source.ObjectAt(0) == object);
			ASSERT_EQ(0, target.CountObjects());
		}
	}

	ASSERT_EQ(1, destructorCount);
}

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
	TestAddObjectsEditRollsBackPartialInsertionFailure();
	TestMoveObjectsEditRestoresSourceOnPartialInsertionFailure();
	TestNestedSelectionNoOpMoveIsRejected();
	printf("edits-move-objects: PASS\n");
	return 0;
}
