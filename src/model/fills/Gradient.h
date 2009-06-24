/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef WB_GRADIENT_H
#define WB_GRADIENT_H


#include <Archivable.h>
#include <GraphicsDefs.h>
#include <List.h>

#include "Notifier.h"
#include "Transformable.h"

class BMessage;

class Gradient : public BArchivable, public Notifier, public Transformable {
public:
	enum Type {
		LINEAR		= 0,
		CIRCULAR	= 1,
		DIAMOND		= 2,
		CONIC		= 3,
		XY			= 4,
		SQRT_XY		= 5
	};

	enum Interpolation {
		BILINEAR	= 0,
		SMOOTH		= 1
	};

	struct ColorStop {
		ColorStop(const rgb_color c, float o);
		ColorStop(uint8 r, uint8 g, uint8 b, uint8 a, float o);
		ColorStop(const ColorStop& other);
		ColorStop();

		bool operator!=(const ColorStop& other) const;

		rgb_color		color;
		float			offset;
	};

public:
								Gradient(bool empty = false);
								Gradient(BMessage* archive);
								Gradient(const Gradient& other);
	virtual						~Gradient();

			status_t			Archive(BMessage* into,
									bool deep = true) const;

			Gradient&			operator=(const Gradient& other);

			bool				operator==(const Gradient& other) const;
			bool				operator!=(const Gradient& other) const;
			bool				ColorStepsAreEqual(
									const Gradient& other) const;

			void				SetColors(const Gradient& other);


			int32				AddColor(const rgb_color& color, float offset);
			bool				AddColor(const ColorStop& color,
									int32 index);

			bool				RemoveColor(int32 index);

			bool				SetColor(int32 index, const ColorStop& step);
			bool				SetColor(int32 index, const rgb_color& color);
			bool				SetOffset(int32 index, float offset);

			int32				CountColors() const;
			ColorStop*			ColorAt(int32 index) const;
			ColorStop*			ColorAtFast(int32 index) const;

			void				SetType(Type type);
			Type				GetType() const
									{ return fType; }

			void				SetInterpolation(Interpolation type);
			Interpolation		GetInterpolation() const
									{ return fInterpolation; }

			void				SetInheritTransformation(bool inherit);
			bool				InheritTransformation() const
									{ return fInheritTransformation; }

			void				MakeGradient(uint32* colors,
									int32 count) const;

			void				FitToBounds(const BRect& bounds);
			BRect				GradientArea() const;
	virtual	void				TransformationChanged();

			void				PrintToStream() const;

private:
			void				_MakeEmpty();

			BList				fColors;
			Type				fType;
			Interpolation		fInterpolation;
			bool				fInheritTransformation;
};

#endif	// WB_GRADIENT_H
