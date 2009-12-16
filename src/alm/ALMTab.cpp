/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2007-2008, Christof Lutteroth, lutteroth@cs.auckland.ac.nz
 * Copyright 2007-2008, James Kim, jkim202@ec.auckland.ac.nz
 * All Rights Reserved. Distributed under the terms of the MIT License.
 */

#include "ALMTab.h"

#include "ALMLayout.h"


/**
 * Constructor.
 */
ALMTab::ALMTab(ALMLayout* ls)
	:
	Variable(ls),
	fPreviousLink(false)
{
}

