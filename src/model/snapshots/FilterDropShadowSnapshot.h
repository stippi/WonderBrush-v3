/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#ifndef FILTER_DROP_SHADOW_SNAPSHOT_H
#define FILTER_DROP_SHADOW_SNAPSHOT_H

#include "ObjectSnapshot.h"

class FilterDropShadow;

class FilterDropShadowSnapshot : public ObjectSnapshot {
 public:
								FilterDropShadowSnapshot(
									const FilterDropShadow* filter);
	virtual						~FilterDropShadowSnapshot();

	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				Layout(LayoutContext& context, uint32 flags);

	virtual	void				Render(RenderEngine& engine,
									RenderBuffer* bitmap, BRect area) const;
	virtual	void				RebuildAreaForDirtyArea(BRect& area) const;

 private:
			const FilterDropShadow*	fOriginal;
			float				fFilterRadius;
			float				fOffsetX;
			float				fOffsetY;
			float				fOpacity;
			float				fLayoutedFilterRadius;
			float				fLayoutedOffsetX;
			float				fLayoutedOffsetY;
};

#endif // FILTER_DROP_SHADOW_SNAPSHOT_H
