#ifndef __MCN_CUSTOM_MATH_H__
#define __MCN_CUSTOM_MATH_H__

#include <math.h>

#define SIGN(X) ((X > 0) - (X < 0))
#define ABS(X) (X * SIGN(X))

__attribute__((__always_inline__)) inline double FABS(double X) {
	*((uint_fast64_t*) &X) &= 0x7FFFFFFFFFFFFFFF;
	return X;
}

int min(int _a, int _b) {
	if (_a < _b) return _a;
	return _b;
}

int max(int _a, int _b) {
	if (_a > _b) return _a;
	return _b;
}

int min3(int _a, int _b, int _c) {
	if (_a < _b)
		if (_a < _c) return _a;
		else return _c;
	else if (_b < _c) return _b;
	else return _c;
}

int max3(int _a, int _b, int _c) {
	if (_a > _b)
		if (_a > _c) return _a;
		else return _c;
	else if (_b > _c) return _b;
	else return _c;
}

int min4(int _a, int _b, int _c, int _d) {
	if (_b < _a)
		if (_c < _b)
			if (_d < _c) return _d;
			else return _c;
		else if (_d < _b) return _d;
		else return _b;
	else if (_c < _a)
		if (_d < _c) return _d;
		else return _c;
	else if (_d < _a) return _d;
	else return _a;
}

int max4(int _a, int _b, int _c, int _d) {
	if (_b > _a)
		if (_c > _b)
			if (_d > _c) return _d;
			else return _c;
		else if (_d > _b) return _d;
		else return _b;
	else if (_c > _a)
		if (_d > _c) return _d;
		else return _c;
	else if (_d > _a) return _d;
	else return _a;
}

#endif