#include "Transformable.h"

Transformable::Transformable()
	: agg::trans_perspective()
{
}

Transformable::Transformable(const Transformable& other)
	: agg::trans_perspective(other)
{
}

Transformable::~Transformable() {}

void Transformable::StoreTo(double matrix[MatrixSize]) const
{
	store_to(matrix);
}

void Transformable::LoadFrom(const double matrix[MatrixSize])
{
	load_from(matrix);
}

void Transformable::SetTransformable(const Transformable& other)
{
	*this = other;
}

Transformable& Transformable::operator=(const Transformable& other)
{
	if (this != &other) {
		reset();
		multiply(other);
	}
	return *this;
}

Transformable& Transformable::Multiply(const Transformable& other)
{
	multiply(other);
	return *this;
}

Transformable& Transformable::PreMultiply(const Transformable& other)
{
	premultiply(other);
	return *this;
}

Transformable& Transformable::MultiplyInverse(const Transformable&)
{
	return *this;
}

void Transformable::Reset() { reset(); }
void Transformable::Invert() { invert(); }
bool Transformable::IsValid() const { return true; }
bool Transformable::IsIdentity() const { return is_identity(); }
bool Transformable::IsTranslationOnly() const { return true; }
bool Transformable::IsNotDistorted() const { return true; }
bool Transformable::IsPerspective() const { return false; }
void Transformable::Transform(double*, double*) const {}
void Transformable::Transform(BPoint*) const {}
BPoint Transformable::Transform(const BPoint& point) const { return point; }
void Transformable::InverseTransform(double*, double*) const {}
void Transformable::InverseTransform(BPoint*) const {}
BPoint Transformable::InverseTransform(const BPoint& point) const { return point; }
BRect Transformable::TransformBounds(BRect bounds) const { return bounds; }
void Transformable::TranslateBy(BPoint) {}
void Transformable::RotateBy(BPoint, double) {}
void Transformable::ScaleBy(BPoint, double, double) {}
double Transformable::Scale() const { return 1.0; }
void Transformable::GetScale(double* scaleX, double* scaleY) const
{
	if (scaleX != 0)
		*scaleX = 1.0;
	if (scaleY != 0)
		*scaleY = 1.0;
}

bool Transformable::GetAffineParameters(double* translationX,
	double* translationY, double* rotation, double* scaleX, double* scaleY,
	double* skewX, double* skewY) const
{
	if (translationX != 0)
		*translationX = 0.0;
	if (translationY != 0)
		*translationY = 0.0;
	if (rotation != 0)
		*rotation = 0.0;
	if (scaleX != 0)
		*scaleX = 1.0;
	if (scaleY != 0)
		*scaleY = 1.0;
	if (skewX != 0)
		*skewX = 0.0;
	if (skewY != 0)
		*skewY = 0.0;
	return true;
}
