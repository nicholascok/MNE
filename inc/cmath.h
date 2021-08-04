#ifndef __MCN_CUSTOM_MATH_H__
#define __MCN_CUSTOM_MATH_H__

#define G 10

#define DSWAPF(X, Y) (*(uint64_t*) &X) ^= (*(uint64_t*) &Y), (*(uint64_t*) &Y) ^= (*(uint64_t*) &X), (*(uint64_t*) &X) ^= (*(uint64_t*) &Y)

#define SIGN(X) (((X) > 0) - ((X) < 0))
#define ABS(X) ((X) * SIGN(X))

__attribute__((__always_inline__)) inline double FABS(double X) {
	*((uint_fast64_t*) &X) &= 0x7FFFFFFFFFFFFFFF;
	return X;
}

__attribute__((__always_inline__)) inline int IFLOOR(double X) {
	int r = (int) X;
	return r - (r > X);
}

__attribute__((__always_inline__)) inline int ICEIL(double X) {
	int r = (int) X;
	return r + (r < X);
}

__attribute__((__always_inline__)) inline double FSQRT(double _x) {
	uint64_t u = (*((uint64_t*) &_x) >> 1) + 0x1FF8000000000000; // initial guess
	*((double*) &u) = 0.5 * (*((double*) &u) + _x / *((double*) &u)); // newton's method (one iteration is suffice)
	return *((double*) &u);
}

__attribute__((__always_inline__)) inline double FISQRT(double _x) {
	uint64_t u = 0x5FE8000000000000 - (*((uint64_t*) &_x) >> 1); // initial guess
	*((double*) &u) *= 1.5 - (0.5 * _x * (*((double*) &u)) * (*((double*) &u))); // newton's method (one iteration is suffice)
	return *((double*) &u);
}

__attribute__((__always_inline__)) inline double FINV(double _x) {
	uint64_t u = 0x7FE0000000000000 - *((uint64_t*) &_x);
	return *((double*) &u);
}

double sin(double _x) {
	double x2 = _x * _x;
	double xp = x2 * _x;
	_x -= xp * 0.16666666666;
	xp *= x2;
	_x += xp * 0.00833333333;
	xp *= x2;
	_x -= xp * 0.00019841269;
	xp *= x2;
	_x += xp * 0.00000275573;
	xp *= x2;
	_x -= xp * 0.00000005052;
	xp *= x2;
	_x += xp * 0.00000000016;
	return _x;
}

double cos(double _x) {
	double x2 = _x * _x;
	double xp = x2;
	_x = 1 - xp * 0.5;
	xp *= x2;
	_x += xp * 0.04166666666;
	xp *= x2;
	_x -= xp * 0.00138888888;
	xp *= x2;
	_x += xp * 0.00002480158;
	xp *= x2;
	_x -= xp * 0.00000027557;
	xp *= x2;
	_x += xp * 0.00000000877;
	return _x;
}

int min(int _a, int _b) {
	return _a + (_b < _a) * (_b - _a);
}

int max(int _a, int _b) {
	return _a + (_b > _a) * (_b - _a);
}

double fmin(double _a, double _b) {
	return _a + (_b < _a) * (_b - _a);
}

double fmax(double _a, double _b) {
	return _a + (_b > _a) * (_b - _a);
}

int min3(int _a, int _b, int _c) {
	_a += (_b < _a) * (_b - _a);
	_a += (_c < _a) * (_c - _a);
	return _a;
}

int max3(int _a, int _b, int _c) {
	_a += (_b > _a) * (_b - _a);
	_a += (_c > _a) * (_c - _a);
	return _a;
}

double fmin3(double _a, double _b, double _c) {
	if (_a < _b && _a < _c) return _a;
	else if (_b < _c) return _b;
	else return _c;
}

double fmax3(double _a, double _b, double _c) {
	if (_a > _b && _a > _c) return _a;
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