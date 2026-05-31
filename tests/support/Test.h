#ifndef WB_TEST_H
#define WB_TEST_H

#include <stdio.h>
#include <stdlib.h>

#define ASSERT_TRUE(expr) \
	do { \
		if (!(expr)) { \
			fprintf(stderr, "ASSERT_TRUE failed at %s:%d: %s\n", \
				__FILE__, __LINE__, #expr); \
			exit(1); \
		} \
	} while (false)

#define ASSERT_EQ(expected, actual) \
	do { \
		auto _expected = (expected); \
		auto _actual = (actual); \
		if (!(_expected == _actual)) { \
			fprintf(stderr, "ASSERT_EQ failed at %s:%d: expected %lld, got %lld\n", \
				__FILE__, __LINE__, (long long)_expected, (long long)_actual); \
			exit(1); \
		} \
	} while (false)

#endif // WB_TEST_H
