#ifndef WB_TEST_STUB_POINT_H
#define WB_TEST_STUB_POINT_H

class BPoint {
public:
	BPoint()
		: x(0)
		, y(0)
	{
	}

	BPoint(float xValue, float yValue)
		: x(xValue)
		, y(yValue)
	{
	}

	float x;
	float y;
};

#endif // WB_TEST_STUB_POINT_H
