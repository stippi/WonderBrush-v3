/*
 * Copyright 2007-2008, Christof Lutteroth, lutteroth@cs.auckland.ac.nz
 * Copyright 2007-2008, James Kim, jkim202@ec.auckland.ac.nz
 * Distributed under the terms of the MIT License.
 */

#ifndef	ALM_TAB_H
#define	ALM_TAB_H

#include <linprog/Variable.h>


class ALMLayout;

/**
 * Defines a grid line (horizontal or vertical).
 */
class ALMTab : public Variable {
protected:
							ALMTab(ALMLayout* ls);

protected:
	/**
	 * Property signifying if there is a constraint which relates
	 * this tab to a different tab which precedes this tab in the layout.
	 * Only used for reverse engineering.
	 */
			bool			fPreviousLink;

public:
	friend class			ALMArea;
	friend class			ALMSpan;
	friend class			ALMLayout;
};

#endif	// ALM_TAB_H
