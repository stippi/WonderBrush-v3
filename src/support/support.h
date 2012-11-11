// support.h

#ifndef SUPPORT_H
#define SUPPORT_H


#include <Rect.h>


class BPositionIO;
class BResources;
class BString;


// constrain
inline void
constrain(float& value, float min, float max)
{
	if (value < min)
		value = min;
	if (value > max)
		value = max;
}

inline int32
constrain_int32_0_255_asm(int32 value)
{
	__asm__ __volatile__ (
		"movl	$0,		%%ecx;"
		"movl	$255,	%%edx;"
		"cmpl	%%ecx,	%%eax;"
		"cmovl	%%ecx,	%%eax;"
		"cmpl	%%edx,	%%eax;"
		"cmovg	%%edx,	%%eax"
		: "=a" (value)
		: "a" (value)
		: "%ecx", "%edx"
	);
	return value;
}


inline int32
constrain_int32_0_255_c(int32 value)
{
    return max_c(0, min_c(255, value));
}


/*!	From FOG library (posted to AGG mailing list without license)
	Multiplies each byte in x by a, where a must by on [0..255].
	Each byte in x is then divided by 255.
*/
inline uint32
multiply_bytes(uint32 x, uint32 a)
{
#if 0 // on 64bit architecture:
	uint64 x0 = ((uint64)x | ((uint64)x << 24)) & uint64(0x00ff00ff00ff00ff);
	x0 *= a;
	x0 = (x0 + ((x0 >> 8) & uint64(0x00ff00ff00ff00ff))
		+ uint64(0x0080008000800080)) >> 8;
	x0 &= uint64(0x00ff00ff00ff00ff);
	return (uint32)(x0 | (x0 >> 24));
#else
	uint32 t0 = ((x & 0x00ff00ff)     ) * a;
	uint32 t1 = ((x & 0xff00ff00) >> 8) * a;

	x  = ((t0 + ((t0 >> 8) & 0x00ff00ff) + 0x00800080) >> 8) & 0x00ff00ff;
	x |= ((t1 + ((t1 >> 8) & 0x00ff00ff) + 0x00800080)     ) & 0xff00ff00;

	return x;
#endif
}


#define constrain_int32_0_255 constrain_int32_0_255_asm

// rect_to_int
inline void
rect_to_int(BRect r, int32& left, int32& top, int32& right, int32& bottom)
{
	left = (int32)floorf(r.left);
	top = (int32)floorf(r.top);
	right = (int32)ceilf(r.right);
	bottom = (int32)ceilf(r.bottom);
}

// point_point_distance
float
point_point_distance(BPoint a, BPoint b);

// point_line_distance
double
point_line_distance(double x1, double y1, double x2, double y2,
	double x,  double y);

// point_line_distance
double
point_line_distance(BPoint point, BPoint a, BPoint b);

// point_stroke_distance
float
point_stroke_distance(BPoint start, BPoint end, BPoint p, float radius);

// calc_angle
double
calc_angle(BPoint origin, BPoint from, BPoint to, bool degree = true);

/*
template <class T>
T min4(const T a, const T b, const T c, const T d)
{
	T e = a < b ? a : b;
	T f = c < d ? c : d;
	return e < f ? e : f;
}
template <class T>
T max4(const T a, const T b, const T c, const T d)
{
	T e = a > b ? a : b;
	T f = c > d ? c : d;
	return e > f ? e : f;
}
*/
float
min4(float a, float b, float c, float d);

inline float
max4(float a, float b, float c, float d)
{
	return max_c(a, max_c(b, max_c(c, d)));
}

inline float
min5(float v1, float v2, float v3, float v4, float v5)
{
	return min_c(min4(v1, v2, v3, v4), v5);
}

inline float
max5(float v1, float v2, float v3, float v4, float v5)
{
	return max_c(max4(v1, v2, v3, v4), v5);
}

inline float
roundf(float v)
{
	if (v >= 0.0)
		return floorf(v + 0.5);
	return ceilf(v - 0.5);
}

double gauss(double f);

void append_float(BString& string, float n, int32 maxDigits = 4);

status_t write_string(BPositionIO* stream, BString& string);

// platform dependent

int32 get_optimal_worker_thread_count();

status_t get_app_resources(BResources& resources);

char* convert_utf16_to_utf8(const void* string, size_t length);

# endif // SUPPORT_H
