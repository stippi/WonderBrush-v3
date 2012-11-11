/*
 * Copyright 2006, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */

#include "support.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <DataIO.h>
#include <Point.h>
#include <String.h>
#include <SupportDefs.h>


// point_point_distance
float
point_point_distance(BPoint a, BPoint b)
{
	float xDiff = b.x - a.x;
	float yDiff = b.y - a.y;
	return sqrtf(xDiff * xDiff + yDiff * yDiff);
}

// point_line_distance
double
point_line_distance(double x1, double y1, double x2, double y2,
	double x,  double y)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	return ((x - x2) * dy - (y - y2) * dx) / sqrt(dx * dx + dy * dy);
}

// point_line_distance
double
point_line_distance(BPoint point, BPoint pa, BPoint pb)
{
	// first figure out if point is between segment start and end points
	double a = point_point_distance(point, pb);
	double b = point_point_distance(point, pa);
	double c = point_point_distance(pa, pb);

	float currentDist = min_c(a, b);

	if (a > 0.0 && b > 0.0) {
		double alpha = acos((b*b + c*c - a*a) / (2*b*c));
		double beta = acos((a*a + c*c - b*b) / (2*a*c));

		if (alpha <= M_PI_2 && beta <= M_PI_2) {
			currentDist = fabs(point_line_distance(pa.x, pa.y, pb.x, pb.y,
												   point.x, point.y));
		}
	}

	return currentDist;
}

// point_stroke_distance
float
point_stroke_distance(BPoint start, BPoint end, BPoint p, float radius)
{
	BRect r(min_c(start.x, end.x), min_c(start.y, end.y),
		max_c(start.x, end.x), max_c(start.y, end.y));
	r.InsetBy(-radius, -radius);

	if (r.Contains(p)) {
		return fabs(point_line_distance(start.x, start.y, end.x, end.y,
			p.x, p.y));
	}

	return min_c(point_point_distance(start, p),
		point_point_distance(end, p));
}


// calc_angle
double
calc_angle(BPoint origin, BPoint from, BPoint to, bool degree)
{
	double angle = 0.0;

	double d = point_line_distance(from.x, from.y,
								   origin.x, origin.y,
								   to.x, to.y);
	if (d != 0.0) {
		double a = point_point_distance(from, to);
		double b = point_point_distance(from, origin);
		double c = point_point_distance(to, origin);
		if (a > 0.0 && b > 0.0 && c > 0.0) {
			angle = acos((b*b + c*c - a*a) / (2.0*b*c));

			if (d < 0.0)
				angle = -angle;

			if (degree)
				angle = angle * 180.0 / M_PI;
		}
	}
	return angle;
}

// min4
float
min4(float a, float b, float c, float d)
{
	return min_c(a, min_c(b, min_c(c, d)));
}

// gauss
double
gauss(double f)
{
	// this aint' a real gauss function
/*	if (f >= -1.0 && f <= 1.0) {
		if (f < -0.5) {
			f = -1.0 - f;
			return (2.0 * f*f);
		}

		if (f < 0.5)
			return (1.0 - 2.0 * f*f);

		f = 1.0 - f;
		return (2.0 * f*f);
	}*/
	if (f > 0.0) {
		if (f < 0.5)
			return (1.0 - 2.0 * f*f);

		f = 1.0 - f;
		return (2.0 * f*f);
	}
	return 1.0;
}

// append_float
void
append_float(BString& string, float n, int32 maxDigits)
{
	int32 rounded = n >= 0.0 ? (int32)fabs(floorf(n)) : (int32)fabs(ceilf(n));

	if (n < 0.0) {
		string << "-";
		n *= -1.0;
	}
	string << rounded;

	if ((float)rounded != n) {
		// find out how many digits remain
		n = n - rounded;
		rounded = (int32)(n * pow(10, maxDigits));
		char tmp[maxDigits + 1];
		sprintf(tmp, "%0*" B_PRId32, (int)maxDigits, rounded);
		tmp[maxDigits] = 0;
		int32 digits = strlen(tmp);
		for (int32 i = strlen(tmp) - 1; i >= 0; i--) {
			if (tmp[i] == '0')
				digits--;
			else
				break;
		}
		// write after decimal
		if (digits > 0) {
			string << ".";
			for (int32 i = 0; i < digits; i++) {
				string << tmp[i];
			}
		}
	}
}


// write_string
status_t
write_string(BPositionIO* stream, BString& string)
{
	if (!stream)
		return B_BAD_VALUE;

	ssize_t written = stream->Write(string.String(), string.Length());
	if (written > B_OK && written < string.Length())
		written = B_ERROR;
	string.SetTo("");
	return written;
}
