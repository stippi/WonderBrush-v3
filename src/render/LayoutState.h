/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef LAYOUT_STATE_H
#define LAYOUT_STATE_H

#include "Style.h"
#include "Transformable.h"

// Represents a graphical state at a certain point during the layout
// of objects.

class LayoutState {
public:
								LayoutState();
								LayoutState(LayoutState* previous);
								~LayoutState();

			LayoutState&		operator=(const LayoutState& other);

			LayoutState*		Previous;

			Transformable		Matrix;

			void				SetFillPaint(Paint* paint);
			void				SetStrokePaint(Paint* paint);
			void				SetStrokeProperties(
									::StrokeProperties* properties);

			const Paint*		FillPaint() const;
			const Paint*		StrokePaint() const;
			const ::StrokeProperties* StrokeProperties() const;

private:
			template <typename MemberType, typename ValueType,
				typename CacheType>
			void				_SetMember(MemberType*& member,
									const ValueType& newValue,
									CacheType& cache);

			SharedPaint*		fFillPaint;
			SharedPaint*		fStrokePaint;
			SharedStrokeProperties*	fStrokeProperties;
};

#endif // LAYOUT_STATE_H
