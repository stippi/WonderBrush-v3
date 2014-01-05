/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef STYLEABLE_SNAPSHOT_H
#define STYLEABLE_SNAPSHOT_H

#include <GraphicsDefs.h>

#include "BoundedObjectSnapshot.h"
#include "Referenceable.h"
#include "RenderEngine.h"

class Styleable;
class Style;


class StyleableSnapshot : public BoundedObjectSnapshot {
public:
								StyleableSnapshot(const Styleable* styleable);
	virtual						~StyleableSnapshot();

	virtual	bool				Sync();

protected:
			void				PrepareRenderEngine(RenderEngine& engine) const;

			Paint*				FillPaint() const
									{ return fFillPaint; }

			Paint*				StrokePaint() const
									{ return fStrokePaint; }

			::StrokeProperties*	StrokeProperties() const
									{ return fStrokeProperties; }

private:
			void				_SyncStyle();

			void				_SetFillPaint(const Paint& paint);
			void				_UnsetFillPaint();

			void				_SetStrokePaint(const Paint& paint);
			void				_UnsetStrokePaint();

			void				_SetStrokeProperties(
									const ::StrokeProperties& properties);
			void				_UnsetStrokeProperties();

			template<typename PropertyType, typename ValueType,
				typename CacheType>
			void				_SetProperty(PropertyType*& member,
									const ValueType& newValue,
									CacheType& cache);

			template<typename PropertyType, typename CacheType>
			void				_UnsetProperty(PropertyType*& member,
									CacheType& cache);

private:
			const Styleable*	fOriginal;

			SharedPaint*		fFillPaint;
			SharedPaint*		fStrokePaint;

			SharedStrokeProperties*	fStrokeProperties;
};

#endif // SHAPE_SNAPSHOT_H
