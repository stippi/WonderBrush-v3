/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef INSPECTOR_VIEW_H
#define INSPECTOR_VIEW_H

#include "Listener.h"
#include "PropertyListView.h"
#include "Selection.h"

class BaseObject;
class EditManager;
class Selection;

class InspectorView : public PropertyListView, public Selection::Listener,
	public Listener {
public:
								InspectorView();
	virtual						~InspectorView();

	// BView interface
	virtual void				MessageReceived(BMessage* message);
	virtual	void				Draw(BRect updateRect);

	// PropertyListView interface
	virtual	void				PropertyChanged(const Property* previous,
									const Property* current);
	virtual	void				PasteProperties(const PropertyObject* object);
	virtual	bool				IsEditingMultipleObjects();

	// Listener interface
	virtual	void				ObjectChanged(const Notifier* object);

	// Selection::Listener interface
	virtual	void				ObjectSelected(const Selectable& selected,
									const Selection::Controller* controller);
	virtual	void				ObjectDeselected(const Selectable& selected,
									const Selection::Controller* controller);

	// InspectorView
			void				SetSelection(Selection* selection);
			void				SetEditManager(EditManager* stack);

private:
			void				_SetObject(BaseObject* object);

			Selection*			fSelection;
			EditManager*		fEditManager;

			BaseObject*			fObject;
			bool				fIgnoreObjectChange;
};

#endif // INSPECTOR_VIEW_H
