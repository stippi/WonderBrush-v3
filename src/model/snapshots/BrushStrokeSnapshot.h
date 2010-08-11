/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef BRUSH_STROKE_SNAPSHOT_H
#define BRUSH_STROKE_SNAPSHOT_H

#include "BrushStroke.h"
#include "ObjectSnapshot.h"

class BrushStrokeSnapshot : public ObjectSnapshot {
public:
								BrushStrokeSnapshot(const BrushStroke* stroke);
	virtual						~BrushStrokeSnapshot();

	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				Render(RenderEngine& engine,
									RenderBuffer* bitmap, BRect area) const;

private:
			void				_Sync();

private:
			const BrushStroke*	fOriginal;
			Brush				fBrush;
			::Stroke			fStroke;
};

#endif // BRUSH_STROKE_SNAPSHOT_H
