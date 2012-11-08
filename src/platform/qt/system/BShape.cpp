#include "BShape.h"

#include <OS.h>


BShape::BShape()
	:
	fPath()
{
}


BShape::BShape(const BShape& other)
	:
	fPath(other.fPath)
{
}


BShape::~BShape()
{
}


BShape&
BShape::operator=(const BShape& other)
{
	fPath = other.fPath;
	return *this;
}


BRect
BShape::Bounds() const
{
	return fPath.boundingRect();
}


BPoint
BShape::CurrentPosition() const
{
	return fPath.currentPosition();
}


status_t
BShape::AddShape(const BShape* other)
{
	fPath.addPath(other->fPath);
	return B_OK;
}


status_t
BShape::MoveTo(BPoint point)
{
	fPath.moveTo(point);
	return B_OK;
}


status_t
BShape::LineTo(BPoint linePoint)
{
	fPath.lineTo(linePoint);
	return B_OK;
}


status_t
BShape::BezierTo(BPoint controlPoints[])
{
	return BezierTo(controlPoints[0], controlPoints[1], controlPoints[2]);
}


status_t
BShape::BezierTo(const BPoint& control1, const BPoint& control2,
	const BPoint& endPoint)
{
	fPath.cubicTo(control1, control2, endPoint);
	return B_OK;
}


status_t
BShape::ArcTo(float rx, float ry, float angle, bool largeArc,
	bool counterClockWise, const BPoint& point)
{
// TODO: QPainterPath::arcTo() can add an arc, but has different parameters.
// They specify the rectangle of the ellipse, start angle, and arc angle, so we
// have to convert them.
debugger("BShape::ArcTo() not implemented yet!");
	return B_ERROR;
}


status_t
BShape::Close()
{
	fPath.closeSubpath();
	return B_OK;
}
