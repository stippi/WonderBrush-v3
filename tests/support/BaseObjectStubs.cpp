#include "BaseObject.h"
#include "CloneContext.h"

#include <Message.h>

// Shared minimal out-of-line implementations for focused tests that link
// BaseObject-derived model objects without pulling in the full UI/property
// stack.
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
