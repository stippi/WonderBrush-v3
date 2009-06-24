#include "Selectable.h"

//#include "PropertyObject.h"

// constructor
Selectable::Selectable()
	: fSelected(false)
{
}

// destructor
Selectable::~Selectable()
{
}

// SetSelected
void
Selectable::SetSelected(bool selected)
{
	if (fSelected != selected) {
		fSelected = selected;
		SelectedChanged();
	}
}

//// GetPropertyObject
//PropertyObject*
//Selectable::GetPropertyObject()
//{
//	return dynamic_cast<PropertyObject*>(this);
//}
