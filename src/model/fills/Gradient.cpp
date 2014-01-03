/*
 * Copyright 2006-2014, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "Gradient.h"

#include <math.h>
#include <stdio.h>

#include <new>

#include <Message.h>

#include "RenderEngine.h"
#include "support.h"


// #pragma mark - Gradient::ColorStop


// constructor
Gradient::ColorStop::ColorStop(const rgb_color c, float o)
{
	color.red = c.red;
	color.green = c.green;
	color.blue = c.blue;
	color.alpha = c.alpha;
	offset = o;
}


// constructor
Gradient::ColorStop::ColorStop(uint8 r, uint8 g, uint8 b, uint8 a, float o)
{
	color.red = r;
	color.green = g;
	color.blue = b;
	color.alpha = a;
	offset = o;
}


// constructor
Gradient::ColorStop::ColorStop(const ColorStop& other)
{
	color.red = other.color.red;
	color.green = other.color.green;
	color.blue = other.color.blue;
	color.alpha = other.color.alpha;
	offset = other.offset;
}


// constructor
Gradient::ColorStop::ColorStop()
{
	color.red = 0;
	color.green = 0;
	color.blue = 0;
	color.alpha = 255;
	offset = 0;
}


// operator!=
bool
Gradient::ColorStop::operator!=(const ColorStop& other) const
{
	return color.red != other.color.red ||
	color.green != other.color.green ||
	color.blue != other.color.blue ||
	color.alpha != other.color.alpha ||
	offset != other.offset;
}


#if 0
static int
sort_color_stops_by_offset(const void* _left, const void* _right)
{
	const Gradient::ColorStop** left = (const Gradient::ColorStop**)_left;
	const Gradient::ColorStop** right = (const Gradient::ColorStop**)_right;
	if ((*left)->offset > (*right)->offset)
		return 1;
	else if ((*left)->offset < (*right)->offset)
		return -1;
	return 0;
}
#endif


// #pragma mark - Gradient


// constructor
Gradient::Gradient(bool empty)
	:
	BArchivable(),
	Notifier(),
	Transformable(),

	fColors(4),
	fType(LINEAR),
	fInterpolation(SMOOTH),
	fInheritTransformation(true)
{
	if (!empty) {
		AddColor(ColorStop(0, 0, 0, 255, 0.0), 0);
		AddColor(ColorStop(255, 255, 255, 255, 1.0), 1);
	}
}

// constructor
Gradient::Gradient(BMessage* archive)
	:
	BArchivable(archive),
	Notifier(),
	Transformable(),

	fColors(4),
	fType(LINEAR),
	fInterpolation(SMOOTH),
	fInheritTransformation(true)
{
	if (archive == NULL)
		return;

	// read transformation
	size_t size = Transformable::MatrixSize;
	const void* matrix;
	ssize_t dataSize = size * sizeof(double);
	if (archive->FindData("transformation", B_DOUBLE_TYPE, &matrix,
			&dataSize) == B_OK
		&& dataSize == (ssize_t)(size * sizeof(double))) {
		LoadFrom((const double*)matrix);
	}

	// color stops
	ColorStop stop;
	for (int32 i = 0; archive->FindFloat("offset", i, &stop.offset) >= B_OK; i++) {
		if (archive->FindInt32("color", i, (int32*)&stop.color) >= B_OK)
			AddColor(stop, i);
		else
			break;
	}
	if (archive->FindInt32("type", (int32*)&fType) < B_OK)
		fType = LINEAR;

	if (archive->FindInt32("interpolation", (int32*)&fInterpolation) < B_OK)
		fInterpolation = SMOOTH;

	if (archive->FindBool("inherit transformation",
						  &fInheritTransformation) < B_OK)
		fInheritTransformation = true;
}

// constructor
Gradient::Gradient(const Gradient& other)
	:
	BArchivable(other),
	Notifier(),
	Transformable(other),

	fColors(4),
	fType(other.fType),
	fInterpolation(other.fInterpolation),
	fInheritTransformation(other.fInheritTransformation)
{
	SetColors(other);
}

// destructor
Gradient::~Gradient()
{
	_MakeEmpty();
}

// Archive
status_t
Gradient::Archive(BMessage* into, bool deep) const
{
	status_t ret = BArchivable::Archive(into, deep);

	// transformation
	if (ret == B_OK) {
		size_t size = Transformable::MatrixSize;
		double matrix[size];
		StoreTo(matrix);
		ret = into->AddData("transformation", B_DOUBLE_TYPE,
			matrix, size * sizeof(double));
	}

	// color stops
	if (ret >= B_OK) {
		for (int32 i = 0; ColorStop* stop = ColorAt(i); i++) {
			ret = into->AddInt32("color", (const uint32&)stop->color);
			if (ret < B_OK)
				break;
			ret = into->AddFloat("offset", stop->offset);
			if (ret < B_OK)
				break;
		}
	}
	// gradient and interpolation type
	if (ret >= B_OK)
		ret = into->AddInt32("type", (int32)fType);
	if (ret >= B_OK)
		ret = into->AddInt32("interpolation", (int32)fInterpolation);
	if (ret >= B_OK)
		ret = into->AddBool("inherit transformation", fInheritTransformation);

	// finish off
	if (ret >= B_OK)
		ret = into->AddString("class", "Gradient");

	return ret;
}

// #pragma mark -

// operator=
Gradient&
Gradient::operator=(const Gradient& other)
{
	AutoNotificationSuspender _(this);

	SetTransformable(other);
	SetColors(other);
	SetType(other.fType);
	SetInterpolation(other.fInterpolation);
	SetInheritTransformation(other.fInheritTransformation);

	return *this;
}

// operator==
bool
Gradient::operator==(const Gradient& other) const
{
	if (Transformable::operator==(other))
		return ColorStepsAreEqual(other);
	return false;
}

// operator!=
bool
Gradient::operator!=(const Gradient& other) const
{
	return !(*this == other);
}

// ColorStepsAreEqual
bool
Gradient::ColorStepsAreEqual(const Gradient& other) const
{
	int32 count = CountColors();
	if (count == other.CountColors() &&
		fType == other.fType &&
		fInterpolation == other.fInterpolation &&
		fInheritTransformation == other.fInheritTransformation) {

		bool equal = true;
		for (int32 i = 0; i < count; i++) {
			ColorStop* ourStep = ColorAtFast(i);
			ColorStop* otherStep = other.ColorAtFast(i);
			if (*ourStep != *otherStep) {
				equal = false;
				break;
			}
		}
		return equal;
	}
	return false;
}

// SetColors
void
Gradient::SetColors(const Gradient& other)
{
	AutoNotificationSuspender _(this);

	_MakeEmpty();
	for (int32 i = 0; ColorStop* stop = other.ColorAt(i); i++)
		AddColor(*stop, i);

	Notify();
}

// #pragma mark -

// AddColor
int32
Gradient::AddColor(const rgb_color& color, float offset)
{
	ColorStop* stop = new (std::nothrow) ColorStop(color, offset);
	if (stop == NULL)
		return -1;

	// find the correct index (sorted by offset)
	int32 index = 0;
	int32 count = CountColors();
	for (; index < count; index++) {
		ColorStop* s = ColorAtFast(index);
		if (s->offset > stop->offset)
			break;
	}

	if (!fColors.AddItem((void*)stop, index)) {
		delete stop;
		return -1;
	}

	Notify();

	return index;
}

// AddColor
bool
Gradient::AddColor(const ColorStop& color, int32 index)
{
	ColorStop* stop = new ColorStop(color);
	if (!fColors.AddItem((void*)stop, index)) {
		delete stop;
		return false;
	}
	Notify();
	return true;
}

// RemoveColor
bool
Gradient::RemoveColor(int32 index)
{
	ColorStop* stop = (ColorStop*)fColors.RemoveItem(index);
	if (stop == NULL)
		return false;

	delete stop;
	Notify();
	return true;
}

// #pragma mark -

// SetColor
bool
Gradient::SetColor(int32 index, const ColorStop& color)
{
	if (ColorStop* stop = ColorAt(index)) {
		if (*stop != color) {
			stop->color = color.color;
			stop->offset = color.offset;
			Notify();
			return true;
		}
	}
	return false;
}

// SetColor
bool
Gradient::SetColor(int32 index, const rgb_color& color)
{
	if (ColorStop* stop = ColorAt(index)) {
		if ((uint32&)stop->color != (uint32&)color) {
			stop->color = color;
			Notify();
			return true;
		}
	}
	return false;
}

// SetOffset
bool
Gradient::SetOffset(int32 index, float offset)
{
	ColorStop* stop = ColorAt(index);
	if (stop && stop->offset != offset) {
		stop->offset = offset;
		Notify();
		return true;
	}
	return false;
}

// #pragma mark -

// CountColors
int32
Gradient::CountColors() const
{
	return fColors.CountItems();
}

// ColorAt
Gradient::ColorStop*
Gradient::ColorAt(int32 index) const
{
	return (ColorStop*)fColors.ItemAt(index);
}

// ColorAtFast
Gradient::ColorStop*
Gradient::ColorAtFast(int32 index) const
{
	return (ColorStop*)fColors.ItemAtFast(index);
}

// #pragma mark -

// SetType
void
Gradient::SetType(Type type)
{
	if (fType != type) {
		fType = type;
		Notify();
	}
}

// SetInterpolation
void
Gradient::SetInterpolation(Interpolation type)
{
	if (fInterpolation != type) {
		fInterpolation = type;
		Notify();
	}
}

// SetInheritTransformation
void
Gradient::SetInheritTransformation(bool inherit)
{
	if (fInheritTransformation != inherit) {
		fInheritTransformation = inherit;
		Notify();
	}
}

// #pragma mark -

// gauss
inline double
gauss(double f)
{
	// this aint' a real gauss function
	if (f > 0.0) {
		if (f < 0.5)
			return (1.0 - 2.0 * f*f);

		f = 1.0 - f;
		return (2.0 * f*f);
	}
	return 1.0;
}

// MakeGradient
void
Gradient::MakeGradient(Color* colors, int32 count) const
{
	ColorStop* from = ColorAt(0);

	if (!from)
		return;

	// find the stop with the lowest offset
	for (int32 i = 0; ColorStop* stop = ColorAt(i); i++) {
		if (stop->offset < from->offset)
			from = stop;
	}

	// current index into "colors" array
	int32 index = (int32)floorf(count * from->offset + 0.5);
	if (index < 0)
		index = 0;
	if (index > count)
		index = count;
	//  make sure we fill the entire array
	if (index > 0) {
		agg::rgba16 c(
			RenderEngine::GammaToLinear(from->color.red),
			RenderEngine::GammaToLinear(from->color.green),
			RenderEngine::GammaToLinear(from->color.blue),
			(from->color.alpha << 8) | from->color.alpha);
		c.premultiply();
		for (int32 i = 0; i < index; i++) {
			colors[i] = c;
		}
	}

	// put all stops that we need to interpolate to into a list
	BList nextSteps(fColors.CountItems() - 1);
	for (int32 i = 0; ColorStop* stop = ColorAt(i); i++) {
		if (stop != from)
			nextSteps.AddItem((void*)stop);
	}

	// interpolate "from" to "to"
	while (!nextSteps.IsEmpty()) {

		// find the stop with the next offset
		ColorStop* to = NULL;
		float nextOffsetDist = 2.0;
		for (int32 i = 0; ColorStop* stop
				= (ColorStop*)nextSteps.ItemAt(i); i++) {
			float d = stop->offset - from->offset;
			if (d < nextOffsetDist && d >= 0) {
				to = stop;
				nextOffsetDist = d;
			}
		}
		if (!to)
			break;

		nextSteps.RemoveItem((void*)to);

		// interpolate
		int32 offset = (int32)floorf((count - 1) * to->offset + 0.5);
		if (offset >= count)
			offset = count - 1;
		int32 dist = offset - index;
		if (dist >= 0) {
			for (int32 i = index; i <= offset; i++) {
				float f = (float)(offset - i) / (float)(dist + 1);
				if (fInterpolation == SMOOTH)
					f = gauss(1.0 - f);

				agg::rgba16 a(
					RenderEngine::GammaToLinear(from->color.red),
					RenderEngine::GammaToLinear(from->color.green),
					RenderEngine::GammaToLinear(from->color.blue),
					(from->color.alpha << 8) | from->color.alpha);
				a.premultiply();

				agg::rgba16 b(
					RenderEngine::GammaToLinear(to->color.red),
					RenderEngine::GammaToLinear(to->color.green),
					RenderEngine::GammaToLinear(to->color.blue),
					(to->color.alpha << 8) | to->color.alpha);
				b.premultiply();
				
				colors[i] = a.gradient(b, f);
			}
		}
		index = offset + 1;
		// the current "to" will be the "from" in the next interpolation
		from = to;
	}
	//  make sure we fill the entire array
	if (index < count) {
		agg::rgba16 c(
			RenderEngine::GammaToLinear(from->color.red),
			RenderEngine::GammaToLinear(from->color.green),
			RenderEngine::GammaToLinear(from->color.blue),
			(from->color.alpha << 8) | from->color.alpha);
		c.premultiply();
		for (int32 i = index; i < count; i++) {
			colors[index] = c;
		}
	}
}

// FitToBounds
void
Gradient::FitToBounds(const BRect& bounds)
{
	double parl[6];
	parl[0] = bounds.left;
	parl[1] = bounds.top;
	parl[2] = bounds.right;
	parl[3] = bounds.top;
	parl[4] = bounds.right;
	parl[5] = bounds.bottom;
	agg::trans_affine transform(-200.0, -200.0, 200.0, 200.0, parl);
	multiply(transform);
}

// string_for_type
static const char*
string_for_type(Gradient::Type type)
{
	switch (type) {
		case Gradient::LINEAR:
			return "LINEAR";
		case Gradient::CIRCULAR:
			return "CIRCULAR";
		case Gradient::DIAMOND:
			return "DIAMOND";
		case Gradient::CONIC:
			return "CONIC";
		case Gradient::XY:
			return "XY";
		case Gradient::SQRT_XY:
			return "SQRT_XY";
	}
	return "<unkown>";
}

//string_for_interpolation
static const char*
string_for_interpolation(Gradient::Interpolation type)
{
	switch (type) {
		case Gradient::BILINEAR:
			return "BILINEAR";
		case Gradient::SMOOTH:
			return "SMOOTH";
	}
	return "<unkown>";
}

// GradientArea
BRect
Gradient::GradientArea() const
{
	BRect area(0, 0, 64, 64);
	switch (fType) {
		case LINEAR:
		case CIRCULAR:
		case DIAMOND:
		case CONIC:
		case XY:
		case SQRT_XY:
			break;
	}
	return area;
}

// TransformationChanged()
void
Gradient::TransformationChanged()
{
	Notify();
}

// PrintToStream
void
Gradient::PrintToStream() const
{
	printf("Gradient: type: %s, interpolation: %s, inherits transform: %d\n",
		   string_for_type(fType),
		   string_for_interpolation(fInterpolation),
		   fInheritTransformation);
	for (int32 i = 0; ColorStop* stop = ColorAt(i); i++) {
		printf("  %" B_PRId32 ": offset: %.1f -> color(%d, %d, %d, %d)\n",
			   i, stop->offset,
			   stop->color.red,
			   stop->color.green,
			   stop->color.blue,
			   stop->color.alpha);
	}
}

// _MakeEmpty
void
Gradient::_MakeEmpty()
{
	int32 count = CountColors();
	for (int32 i = 0; i < count; i++)
		delete ColorAtFast(i);
	fColors.MakeEmpty();
}
