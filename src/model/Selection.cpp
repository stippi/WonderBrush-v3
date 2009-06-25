/*
 * Copyright 2007-2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "Selection.h"

#include <stdio.h>

#include <debugger.h>

#include "Selectable.h"

#undef DEBUG
#define DEBUG 1


static const Selectable kEmptySelectable;


// constructor
Selection::Listener::Listener()
{
}

// destructor
Selection::Listener::~Listener()
{
}


// #pragma mark -

// constructor
Selection::Controller::Controller()
{
}

// destructor
Selection::Controller::~Controller()
{
}


// #pragma mark -

// constructor
Selection::Selection()
	:
	fSelected(),
	fListeners(4)
{
}

// destructor
Selection::~Selection()
{
}

// Select
bool
Selection::Select(const Selectable& object, const Controller* controller,
	bool extend)
{
	if (!extend)
		_DeselectAllExcept(object, controller);

	Container::iterator it = fSelected.begin();
	for (; it != fSelected.end(); ++it) {
		if (*it == object)
			return true;
	}

	try {
		fSelected.push_back(object);
		_NotifyObjectSelected(object, controller);
		return true;
	} catch (...) {
	}

	return false;
}

// Deselect
void
Selection::Deselect(const Selectable& object, const Controller* controller)
{
	try {
		Container::iterator it = fSelected.begin();
		for (; it != fSelected.end(); ++it) {
			if (*it == object) {
				fSelected.erase(it);
				_NotifyObjectDeselected(object, controller);
				return;
			}
		}
	} catch (...) {
	}
}

// DeselectAll
void
Selection::DeselectAll(const Controller* controller)
{
	_DeselectAllExcept(kEmptySelectable, controller);
}

// #pragma mark -

// SelectableAt
const Selectable&
Selection::SelectableAt(uint32 index) const
{
	if (index < fSelected.size())
		return fSelected[index];

	return kEmptySelectable;
}

// SelectableAtFast
const Selectable&
Selection::SelectableAtFast(uint32 index) const
{
	return fSelected[index];
}

// CountSelected
uint32
Selection::CountSelected() const
{
	return fSelected.size();
}

// IsEmpty
bool
Selection::IsEmpty() const
{
	return fSelected.empty();
}

// #pragma mark -

// AddListener
bool
Selection::AddListener(Listener* listener)
{
	if (listener == NULL || fListeners.HasItem(listener))
		return false;
	return fListeners.AddItem(listener);
}

// RemoveListener
void
Selection::RemoveListener(Listener* listener)
{
	fListeners.RemoveItem(listener);
}

// #pragma mark -

// _DeselectAllExcept
void
Selection::_DeselectAllExcept(const Selectable& except,
	const Controller* controller)
{
	bool containedExcept = false;

	// Make a temporary copy of the list and read the selected objects
	// from there. Make the original list empty here already, in order to
	// prevent potential screw-ups when someone selects objects in his
	// notification hooks...
	Container selected;
	selected.assign(fSelected.begin(), fSelected.end());
	fSelected.clear();

	int32 count = selected.size();
	for (int32 i = 0; i < count; i++) {
		const Selectable& object = selected[i];
		if (object != except)
			_NotifyObjectDeselected(object, controller);
		else
			containedExcept = true;
	}

	// if the "except" object was previously
	// in the selection, add it again after
	// making the selection list empty
	if (containedExcept)
		fSelected.push_back(except);
}

// _NotifyObjectSelected
void
Selection::_NotifyObjectSelected(const Selectable& object,
	const Controller* controller)
{
	BList listeners(fListeners);
	int32 count = listeners.CountItems();
	for (int32 i = 0; i < count; i++) {
		((Listener*)listeners.ItemAtFast(i))->ObjectSelected(object,
			controller);
	}
}

// _NotifyObjectDeselected
void
Selection::_NotifyObjectDeselected(const Selectable& object,
	const Controller* controller)
{
	BList listeners(fListeners);
	int32 count = listeners.CountItems();
	for (int32 i = 0; i < count; i++) {
		((Listener*)listeners.ItemAtFast(i))->ObjectDeselected(object,
			controller);
	}
}


