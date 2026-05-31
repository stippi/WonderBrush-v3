#ifndef WB_TEST_STUB_RECT_H
#define WB_TEST_STUB_RECT_H

#include "Point.h"

class BRect {
public:
	BRect()
		: left(0)
		, top(0)
		, right(-1)
		, bottom(-1)
	{
	}

	BRect(float l, float t, float r, float b)
		: left(l)
		, top(t)
		, right(r)
		, bottom(b)
	{
	}

	bool IsValid() const
	{
		return left <= right && top <= bottom;
	}

	bool operator==(const BRect& other) const
	{
		return left == other.left && top == other.top && right == other.right
			&& bottom == other.bottom;
	}

	bool operator!=(const BRect& other) const
	{
		return !(*this == other);
	}

	float left;
	float top;
	float right;
	float bottom;
};

#endif // WB_TEST_STUB_RECT_H
