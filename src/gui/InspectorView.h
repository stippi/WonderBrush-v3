/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef INSPECTOR_VIEW_H
#define INSPECTOR_VIEW_H

#include "PropertyListView.h"

class InspectorView : public PropertyListView {
public:
								InspectorView();
	virtual						~InspectorView();

	// PropertyListView interface
	virtual	void				PropertyChanged(const Property* previous,
									const Property* current);
	virtual	void				PasteProperties(const PropertyObject* object);
	virtual	bool				IsEditingMultipleObjects();

};

#endif // INSPECTOR_VIEW_H
