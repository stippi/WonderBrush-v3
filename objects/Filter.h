/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef FILTER_H
#define FILTER_H

#include "Object.h"

class Filter : public Object {
 public:
								Filter();
								Filter(float radius);
	virtual						~Filter();

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	void				ExtendDirtyArea(BRect& area) const;

	// Filter
	inline	float				FilterRadius() const
									{ return fFilterRadius; }

 private:
 			float				fFilterRadius;
};

#endif // FILTER_H
