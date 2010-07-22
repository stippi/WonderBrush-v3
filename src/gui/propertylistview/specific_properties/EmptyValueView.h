/*
 * Copyright 2006-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef EMPTY_VALUE_VIEW_H
#define EMPTY_VALUE_VIEW_H

#include "Property.h"
#include "PropertyEditorView.h"

class EmptyValueView : public PropertyEditorView {
public:
								EmptyValueView(Property* property);
	virtual						~EmptyValueView();

	// BView interface
	virtual	void				Draw(BRect updateRect);

	virtual	void				MouseDown(BPoint where);

	// PropertyEditorView interface
	virtual	void				SetEnabled(bool enabled);

	virtual	bool				AdoptProperty(Property* property);
	virtual	Property*			GetProperty() const;

private:
			Property*			fProperty;
};

#endif // EMPTY_VALUE_VIEW_H


