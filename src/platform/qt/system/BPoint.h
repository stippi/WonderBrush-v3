/*
 * Copyright 2001-2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef	_POINT_H
#define	_POINT_H


#include <SupportDefs.h>

#include <QPoint>
#include <QPointF>


class BRect;


class BPoint {
public:
			float				x;
			float				y;

								BPoint();
								BPoint(float x, float y);
								BPoint(const BPoint& p);
		
			BPoint&				operator=(const BPoint& other);
			void				Set(float x, float y);

			void				ConstrainTo(BRect rect);
			void				PrintToStream() const;
			
			BPoint				operator-() const;
			BPoint				operator+(const BPoint& other) const;
			BPoint				operator-(const BPoint& other) const;
			BPoint&				operator+=(const BPoint& other);
			BPoint&				operator-=(const BPoint& other);

			bool				operator!=(const BPoint& other) const;
			bool				operator==(const BPoint& other) const;

			QPoint				ToQPoint() const;
	static	BPoint				FromQPoint(const QPoint& qPoint);

								operator QPointF() const;
};


extern const BPoint B_ORIGIN;
	// returns (0, 0)


inline
BPoint::BPoint()
	:
	x(0.0f),
	y(0.0f)
{
}


inline
BPoint::BPoint(float x, float y)
	:
	x(x),
	y(y)
{
}


inline
BPoint::BPoint(const BPoint& other)
	:
	x(other.x),
	y(other.y)
{
}


inline BPoint&
BPoint::operator=(const BPoint& other)
{
	x = other.x;
	y = other.y;
	return *this;
}


inline void
BPoint::Set(float x, float y)
{
	this->x = x;
	this->y = y;
}


inline QPoint
BPoint::ToQPoint() const
{
	return QPoint(x, y);
}


/*static*/ inline BPoint
BPoint::FromQPoint(const QPoint& qPoint)
{
	return BPoint(qPoint.x(), qPoint.y());
}


inline
BPoint::operator QPointF() const
{
	return QPointF(x, y);
}



#endif // _POINT_H
