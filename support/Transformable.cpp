// Transformable.h

#include <stdio.h>
#include <string.h>

#include "support.h"

#include "Transformable.h"

// constructor
Transformable::Transformable()
	: agg::trans_perspective()
{
}

// copy constructor
Transformable::Transformable(const Transformable& other)
	: agg::trans_perspective(other)
{
}

// destructor
Transformable::~Transformable()
{
}

// StoreTo
void
Transformable::StoreTo(double matrix[9]) const
{
	store_to(matrix);
}

// LoadFrom
void
Transformable::LoadFrom(double matrix[9])
{
	// before calling the potentially heavy TransformationChanged()
	// hook function, we make sure that the transformation really changed
	Transformable t;
	t.load_from(matrix);
	if (*this != t) {
		load_from(matrix);
		TransformationChanged();
	}
}

// SetTransformable
void
Transformable::SetTransformable(const Transformable& other)
{
	if (*this != other) {
		*this = other;
		TransformationChanged();
	}
}

// operator=
Transformable&
Transformable::operator=(const Transformable& other)
{
	if (other != *this) {
		reset();
		multiply(other);
		TransformationChanged();
	}
	return *this;
}

// Multiply
Transformable&
Transformable::Multiply(const Transformable& other)
{
	if (!other.IsIdentity()) {
		multiply(other);
		TransformationChanged();
	}
	return *this;
}

// Reset
void
Transformable::Reset()
{
	reset();
}

// Invert
void
Transformable::Invert()
{
	invert();
}

// IsIdentity
bool
Transformable::IsIdentity() const
{
	return is_identity();
}

// IsTranslationOnly
bool
Transformable::IsTranslationOnly() const
{
	if (sx == 1.0 &&
		shy == 0.0 &&
		shx == 0.0 &&
		sy == 1.0 &&
		w0 == 0.0 &&
		w1 == 0.0 &&
		w2 == 1.0)
		return true;
	return false;
}

// IsNotDistorted
bool
Transformable::IsNotDistorted() const
{
	return sx == sy && w0 == 0.0 && w1 == 0.0 && w2 == 1.0;
}

// IsValid
bool
Transformable::IsValid() const
{
	return ((sx * sy - shy * shx) != 0.0);
}

// Transform
void
Transformable::Transform(double* x, double* y) const
{
	transform(x, y);
}

// Transform
void
Transformable::Transform(BPoint* point) const
{
	if (point) {
		double x = point->x;
		double y = point->y;

		transform(&x, &y);
	
		point->x = x;
		point->y = y;
	}
}

// Transform
BPoint
Transformable::Transform(const BPoint& point) const
{
	BPoint p(point);
	Transform(&p);
	return p;
}

// InverseTransform
void
Transformable::InverseTransform(double* x, double* y) const
{
	inverse_transform(x, y);
}

// InverseTransform
void
Transformable::InverseTransform(BPoint* point) const
{
	if (point) {
		double x = point->x;
		double y = point->y;

		inverse_transform(&x, &y);
	
		point->x = x;
		point->y = y;
	}
}

// InverseTransform
BPoint
Transformable::InverseTransform(const BPoint& point) const
{
	BPoint p(point);
	InverseTransform(&p);
	return p;
}

// TransformBounds
BRect
Transformable::TransformBounds(BRect bounds) const
{
	if (bounds.IsValid()) {
		BPoint lt(bounds.left, bounds.top);
		BPoint rt(bounds.right, bounds.top);
		BPoint lb(bounds.left, bounds.bottom);
		BPoint rb(bounds.right, bounds.bottom);
	
		Transform(&lt);
		Transform(&rt);
		Transform(&lb);
		Transform(&rb);
	
		return BRect(floorf(min4(lt.x, rt.x, lb.x, rb.x)),
					 floorf(min4(lt.y, rt.y, lb.y, rb.y)),
					 ceilf(max4(lt.x, rt.x, lb.x, rb.x)),
					 ceilf(max4(lt.y, rt.y, lb.y, rb.y)));
	}
	return bounds;
}

// TranslateBy
void
Transformable::TranslateBy(BPoint offset)
{
	if (offset.x != 0.0 || offset.y != 0.0) {
		multiply(agg::trans_affine_translation(offset.x, offset.y));
		TransformationChanged();
	}
}

// RotateBy
void
Transformable::RotateBy(BPoint origin, double degrees)
{
	if (degrees != 0.0) {
		multiply(agg::trans_affine_translation(-origin.x, -origin.y));
		multiply(agg::trans_affine_rotation(degrees * (PI / 180.0)));
		multiply(agg::trans_affine_translation(origin.x, origin.y));
		TransformationChanged();
	}
}

// ScaleBy
void
Transformable::ScaleBy(BPoint origin, double xScale, double yScale)
{
	if (xScale != 1.0 || yScale != 1.0) {
		multiply(agg::trans_affine_translation(-origin.x, -origin.y));
		multiply(agg::trans_affine_scaling(xScale, yScale));
		multiply(agg::trans_affine_translation(origin.x, origin.y));
		TransformationChanged();
	}
}
