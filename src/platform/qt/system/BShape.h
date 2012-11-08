/*
 * Copyright 2006-2007, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef PLATFORM_QT_SHAPE_H
#define PLATFORM_QT_SHAPE_H


#include <Rect.h>

#include <QPainterPath>


class BShape {
public:
								BShape();
								BShape(const BShape& other);
								~BShape();

			BShape&				operator=(const BShape& other);

			bool				operator==(const BShape& other) const
									{ return fPath == other.fPath; }
			bool				operator!=(const BShape& other) const
									{ return !(*this == other); }

			void				Clear()
									{ fPath = QPainterPath(); }
			BRect				Bounds() const;
			BPoint				CurrentPosition() const;

			status_t			AddShape(const BShape* other);

			status_t			MoveTo(BPoint point);
			status_t			LineTo(BPoint linePoint);
			status_t			BezierTo(BPoint controlPoints[3]);
			status_t			BezierTo(const BPoint& control1,
									const BPoint& control2,
									const BPoint& endPoint);
			status_t			ArcTo(float rx, float ry,
									float angle, bool largeArc,
									bool counterClockWise,
									const BPoint& point);
			status_t			Close();

			QPainterPath&		PainterPath()
									{ return fPath; }
			const QPainterPath&	PainterPath() const
									{ return fPath; }

private:
			QPainterPath		fPath;
};


#endif // PLATFORM_QT_SHAPE_H
