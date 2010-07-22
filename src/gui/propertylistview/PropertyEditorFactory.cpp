/*
 * Copyright 2006-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "PropertyEditorFactory.h"

#include "ColorProperty.h"
#include "Property.h"
#include "IconProperty.h"
#include "Int64Property.h"
#include "OptionProperty.h"
#include "PropertyObjectProperty.h"

#include "BoolValueView.h"
#include "ColorValueView.h"
#include "EmptyValueView.h"
#include "FloatValueView.h"
#include "IconValueView.h"
#include "IntValueView.h"
#include "Int64ValueView.h"
#include "OptionValueView.h"
#include "StringValueView.h"

PropertyEditorView*
EditorFor(Property* p)
{
	if (!p)
		return NULL;

	if (IntProperty* i = dynamic_cast<IntProperty*>(p))
		return new IntValueView(i);

	if (FloatProperty* f = dynamic_cast<FloatProperty*>(p))
		return new FloatValueView(f);

	if (BoolProperty* b = dynamic_cast<BoolProperty*>(p))
		return new BoolValueView(b);

	if (StringProperty* s = dynamic_cast<StringProperty*>(p))
		return new StringValueView(s);

	if (Int64Property* i = dynamic_cast<Int64Property*>(p))
		return new Int64ValueView(i);

	if (OptionProperty* o = dynamic_cast<OptionProperty*>(p))
		return new OptionValueView(o);

	if (ColorProperty* c = dynamic_cast<ColorProperty*>(p))
		return new ColorValueView(c);

	if (IconProperty* i = dynamic_cast<IconProperty*>(p)) {
		IconValueView* view = new IconValueView(i);
		view->SetIcon(i->Icon(), i->Width(), i->Height(), i->Format());
		return view;
	}

	if (dynamic_cast<PropertyObjectProperty*>(p) != NULL)
		return new EmptyValueView(p);

	return NULL;
}

