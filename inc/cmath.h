#ifndef __CORDAC_MATH_H__
#define __CORDAC_MATH_H__

int abs(int _x) {
	return _x - 2 * (_x < 0) * _x;
}

double dabs(double _x) {
	return _x - 2 * (_x < 0) * _x;
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

double dmin4(double _a, double _b, double _c, double _d) {
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