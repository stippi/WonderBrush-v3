/*
 * Copyright 2007-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef STYLEABLE_SNAPSHOT_H
#define STYLEABLE_SNAPSHOT_H

#include <GraphicsDefs.h>

#include "ObjectSnapshot.h"
#include "Referenceable.h"
#include "RenderEngine.h"

class Styleable;
class Style;


class StyleableSnapshot : public ObjectSnapshot {
public:
								StyleableSnapshot(const Styleable* styleable);
	virtual						~StyleableSnapshot();

	virtual	bool				Sync();

	virtual	void				Render(RenderEngine& engine, BBitmap* bitmap,
									BRect area) const;

private:
			const Styleable*	fOriginal;
protected:
			Reference<Style>	fStyle;
};

#endif // SHAPE_SNAPSHOT_H
