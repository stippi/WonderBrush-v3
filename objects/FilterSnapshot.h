/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef FILTER_CLONE_H
#define FILTER_CLONE_H

#include "ObjectSnapshot.h"

class Filter;

class FilterSnapshot : public ObjectSnapshot {
 public:
								FilterSnapshot(const Filter* filter);
	virtual						~FilterSnapshot();

	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				Render(BBitmap* bitmap, BRect area) const;
	virtual	void				ExtendDirtyArea(BRect& area) const;
	virtual	void				RebuildAreaForDirtyArea(BRect& area) const;

 private:
			const Filter*		fOriginal;
			float				fFilterRadius;
};

#endif // FILTER_CLONE_H
