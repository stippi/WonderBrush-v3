/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef FILTER_SATURATION_SNAPSHOT_H
#define FILTER_SATURATION_SNAPSHOT_H

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

#endif // FILTER_SATURATION_SNAPSHOT_H
