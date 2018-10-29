/*
 * Copyright 2018, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef FILTER_BRIGHTNESS_SNAPSHOT_H
#define FILTER_BRIGHTNESS_SNAPSHOT_H

#include "ObjectSnapshot.h"

class FilterBrightness;

class FilterBrightnessSnapshot : public ObjectSnapshot {
 public:
								FilterBrightnessSnapshot(
									const FilterBrightness* filter);
	virtual						~FilterBrightnessSnapshot();

	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				Render(RenderEngine& engine,
									RenderBuffer* bitmap, BRect area) const;

 private:
			const FilterBrightness*		fOriginal;
			int32				fOffset;
			float				fFactor;
};

#endif // FILTER_BRIGHTNESS_SNAPSHOT_H
