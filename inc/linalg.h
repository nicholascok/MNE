#ifndef __MCN_LINEAR_ALGEBRA_H__
#define __MCN_LINEAR_ALGEBRA_H__

typedef struct {
	double get[16];
} mat4;

typedef struct {
	double get[9];
} mat3;

typedef struct {
	int x, y;
} vec2i;

typedef struct {
	int x, y, z;
} vec3i;

typedef struct {
	double x, y, z;
} vec3;

typedef struct {
	double x, y, z, w;
} vec4;

mat3 mT3(mat3 _m) {
	return (mat3) {{
		_m.get[0], _m.get[3], _m.get[6],
		_m.get[1], _m.get[4], _m.get[7],
		_m.get[2], _m.get[5], _m.get[8],
	}};
}

/*
	applies inverse affine transform to vector by using the fact that an affine transform can be representes as follows:
	
		[A  b]
		[0  1]
	
	where A is a 3 by 3 rotation matrix (which is orthogonal), and b is a 3 length column vector (representing translation).
	By the inverse of a 2 by 2 matrix, the inverse of this matrix is as follows:
	
		inv([A  b]) = 1 / (A * 1 - b * 0) * [1  -b] = 1 / A * [1  -b] = inv(A) * [1  -b] = [inv(A)  -inv(A) * b]
		   ([0  1])                         [0   A]           [0   A]            [0   A]   [  0          1     ]
	
	since A is orthogonal, its inverse is given by its transpose (denoted by '), thus our previous expression is now:
	
		inv([A  b]) = [A'  -A' * b]
		   ([0  1])   [0      1   ]
	
	if we are multiplying by a 3-length column vector v, then this becomes:
	
		[A'  -A' * b] * [v] = [A' * v + (-A') * b] = [A' * v - A' * b] = [A' * (v - b)]
		[0      1   ]   [1]   [        1         ]   [       1       ]   [     1      ]
	
	parameters:
	_m is the (non-inverted) 4 by 4 affine matrix, and _v is the vector to be multiplied.
*/
vec3 mmul_inv_affine(mat4* _m, vec3 _v) {
	_v.x -= _m->get[3 ];
	_v.y -= _m->get[7 ];
	_v.z -= _m->get[11];
	
	return (vec3) {
		_m->get[0] * _v.x + _m->get[4] * _v.y + _m->get[8 ] * _v.z,
		_m->get[1] * _v.x + _m->get[5] * _v.y + _m->get[9 ] * _v.z,
		_m->get[2] * _v.x + _m->get[6] * _v.y + _m->get[10] * _v.z,
	};
}

vec3 mmul_inv_affine_no_translation(mat4* _m, vec3 _v) {
	return (vec3) {
		_m->get[0] * _v.x + _m->get[4] * _v.y + _m->get[8 ] * _v.z,
		_m->get[1] * _v.x + _m->get[5] * _v.y + _m->get[9 ] * _v.z,
		_m->get[2] * _v.x + _m->get[6] * _v.y + _m->get[10] * _v.z,
	};
}

vec4 mmul_vec4(mat4* _m, vec4 _v) {
	return (vec4) {
		_m->get[0 ] * _v.x + _m->get[1 ] * _v.y + _m->get[2 ] * _v.z + _m->get[3 ] * _v.w,
		_m->get[4 ] * _v.x + _m->get[5 ] * _v.y + _m->get[6 ] * _v.z + _m->get[7 ] * _v.w,
		_m->get[8 ] * _v.x + _m->get[9 ] * _v.y + _m->get[10] * _v.z + _m->get[11] * _v.w,
		_m->get[12] * _v.x + _m->get[13] * _v.y + _m->get[14] * _v.z + _m->get[15] * _v.w,
	};
}

vec3 mmul_vec3(mat3 _m, vec3 _v) {
	return (vec3) {
		_m.get[0] * _v.x + _m.get[1] * _v.y + _m.get[2] * _v.z,
		_m.get[3] * _v.x + _m.get[4] * _v.y + _m.get[5] * _v.z,
		_m.get[6] * _v.x + _m.get[7] * _v.y + _m.get[8] * _v.z,
	};
}

void mmul4(mat4* _r, mat4* _a, mat4* _b) {
	*_r = (mat4) {{
		_a->get[0] * _b->get[0 ] + _a->get[4] * _b->get[1 ] + _a->get[8 ] * _b->get[2 ] + _a->get[12] * _b->get[3 ],
		_a->get[1] * _b->get[0 ] + _a->get[5] * _b->get[1 ] + _a->get[9 ] * _b->get[2 ] + _a->get[13] * _b->get[3 ],
		_a->get[2] * _b->get[0 ] + _a->get[6] * _b->get[1 ] + _a->get[10] * _b->get[2 ] + _a->get[14] * _b->get[3 ],
		_a->get[3] * _b->get[0 ] + _a->get[7] * _b->get[1 ] + _a->get[11] * _b->get[2 ] + _a->get[15] * _b->get[3 ],
		_a->get[0] * _b->get[4 ] + _a->get[4] * _b->get[5 ] + _a->get[8 ] * _b->get[6 ] + _a->get[12] * _b->get[7 ],
		_a->get[1] * _b->get[4 ] + _a->get[5] * _b->get[5 ] + _a->get[9 ] * _b->get[6 ] + _a->get[13] * _b->get[7 ],
		_a->get[2] * _b->get[4 ] + _a->get[6] * _b->get[5 ] + _a->get[10] * _b->get[6 ] + _a->get[14] * _b->get[7 ],
		_a->get[3] * _b->get[4 ] + _a->get[7] * _b->get[5 ] + _a->get[11] * _b->get[6 ] + _a->get[15] * _b->get[7 ],
		_a->get[0] * _b->get[8 ] + _a->get[4] * _b->get[9 ] + _a->get[8 ] * _b->get[10] + _a->get[12] * _b->get[11],
		_a->get[1] * _b->get[8 ] + _a->get[5] * _b->get[9 ] + _a->get[9 ] * _b->get[10] + _a->get[13] * _b->get[11],
		_a->get[2] * _b->get[8 ] + _a->get[6] * _b->get[9 ] + _a->get[10] * _b->get[10] + _a->get[14] * _b->get[11],
		_a->get[3] * _b->get[8 ] + _a->get[7] * _b->get[9 ] + _a->get[11] * _b->get[10] + _a->get[15] * _b->get[11],
		_a->get[0] * _b->get[12] + _a->get[4] * _b->get[13] + _a->get[8 ] * _b->get[14] + _a->get[12] * _b->get[15],
		_a->get[1] * _b->get[12] + _a->get[5] * _b->get[13] + _a->get[9 ] * _b->get[14] + _a->get[13] * _b->get[15],
		_a->get[2] * _b->get[12] + _a->get[6] * _b->get[13] + _a->get[10] * _b->get[14] + _a->get[14] * _b->get[15],
		_a->get[3] * _b->get[12] + _a->get[7] * _b->get[13] + _a->get[11] * _b->get[14] + _a->get[15] * _b->get[15],
	}};
}

double dot3(vec3 _a, vec3 _b) {
	return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z;
}

vec3 cross3(vec3 _a, vec3 _b) {
	return (vec3) {_a.y * _b.z - _a.z * _b.y, _a.x * _b.z - _a.z * _b.x, _a.x * _b.y - _a.y * _b.x};
}

vec3 norm3(vec3 _v) {
	if ((!_v.x) & (!_v.y) & (!_v.y)) return (vec3) {0};
	double mag = FISQRT(_v.x * _v.x + _v.y * _v.y + _v.z * _v.z);
	return (vec3) {_v.x * mag, _v.y * mag, _v.z * mag};
}

double dot4(vec4 _a, vec4 _b) {
	return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z + _a.w * _b.w;
}

#endif