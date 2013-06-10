/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef FILTER_CLONE_H
#define FILTER_CLONE_H

#include "ObjectSnapshot.h"

class FilterSaturation;

class FilterSaturationSnapshot : public ObjectSnapshot {
 public:
								FilterSaturationSnapshot(
									const FilterSaturation* filter);
	virtual						~FilterSaturationSnapshot();

	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				Render(RenderEngine& engine,
									RenderBuffer* bitmap, BRect area) const;

 private:
			const FilterSaturation*		fOriginal;
			float				fSaturation;
};

#endif // FILTER_CLONE_H
