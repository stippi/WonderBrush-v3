/*
 * Copyright 2018, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef FILTER_CONTRAST_SNAPSHOT_H
#define FILTER_CONTRAST_SNAPSHOT_H

#include "ObjectSnapshot.h"

class FilterContrast;

class FilterContrastSnapshot : public ObjectSnapshot {
 public:
								FilterContrastSnapshot(
									const FilterContrast* filter);
	virtual						~FilterContrastSnapshot();

	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				Render(RenderEngine& engine,
									RenderBuffer* bitmap, BRect area) const;

 private:
			const FilterContrast*		fOriginal;
			float				fContrast;
			uint8				fCenter;
};

#endif // FILTER_CONTRAST_SNAPSHOT_H
