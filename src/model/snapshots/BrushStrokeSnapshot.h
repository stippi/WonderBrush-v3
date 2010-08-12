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
			float				_StepDist(float scale) const;
			bool				_StrokeLine(const StrokePoint* a,
									const StrokePoint* b, uint8* dest,
									uint32 bpr, const BRect& constrainRect,
									float& stepDistLeftOver) const;

private:
			const BrushStroke*	fOriginal;
			Brush				fBrush;
			::Stroke			fStroke;

			// TODO: Move these into Brush
			float				fMinAlpha;
			float				fMaxAlpha;
			float				fMaxSpacing;
			uint32				fFlags;
//			float				fStepDistLeftOver;
};

#endif // BRUSH_STROKE_SNAPSHOT_H
