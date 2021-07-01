#ifndef __CORDAC_LINEAR_ALGEBRA_H__
#define __CORDAC_LINEAR_ALGEBRA_H__

typedef struct {
	double get[4][4];
} mat4;

typedef struct {
	double get[4][4];
} mat3;

typedef struct {
	double x, y, z;
} vec3;

typedef struct {
	int x, y, z;
} vec3i;

typedef struct {
	double x, y, z, w;
} vec4;

vec4 mmul_vec4(mat4 _m, vec4 _v) {
	return (vec4) {
		_m.get[0][0] * _v.x + _m.get[0][1] * _v.y + _m.get[0][2] * _v.z + _m.get[0][3] * _v.w,
		_m.get[1][0] * _v.x + _m.get[1][1] * _v.y + _m.get[1][2] * _v.z + _m.get[1][3] * _v.w,
		_m.get[2][0] * _v.x + _m.get[2][1] * _v.y + _m.get[2][2] * _v.z + _m.get[2][3] * _v.w,
		_m.get[3][0] * _v.x + _m.get[3][1] * _v.y + _m.get[3][2] * _v.z + _m.get[3][3] * _v.w,
	};
}

vec3 mmul_vec3(mat3 _m, vec3 _v) {
	return (vec3) {
		_m.get[0][0] * _v.x + _m.get[0][1] * _v.y + _m.get[0][2],
		_m.get[1][0] * _v.x + _m.get[1][1] * _v.y + _m.get[1][2],
		_m.get[2][0] * _v.x + _m.get[2][1] * _v.y + _m.get[2][2],
	};
}

mat4 mmul4(mat4 _a, mat4 _b) {
	return (mat4) {{
		{
			_a.get[0][0] * _b.get[0][0] + _a.get[1][0] * _b.get[0][1] + _a.get[2][0] * _b.get[0][2] + _a.get[3][0] * _b.get[0][3],
			_a.get[0][1] * _b.get[0][0] + _a.get[1][1] * _b.get[0][1] + _a.get[2][1] * _b.get[0][2] + _a.get[3][1] * _b.get[0][3],
			_a.get[0][2] * _b.get[0][0] + _a.get[1][2] * _b.get[0][1] + _a.get[2][2] * _b.get[0][2] + _a.get[3][2] * _b.get[0][3],
			_a.get[0][3] * _b.get[0][0] + _a.get[1][3] * _b.get[0][1] + _a.get[2][3] * _b.get[0][2] + _a.get[3][3] * _b.get[0][3],
		},
		{
			_a.get[0][0] * _b.get[1][0] + _a.get[1][0] * _b.get[1][1] + _a.get[2][0] * _b.get[1][2] + _a.get[3][0] * _b.get[1][3],
			_a.get[0][1] * _b.get[1][0] + _a.get[1][1] * _b.get[1][1] + _a.get[2][1] * _b.get[1][2] + _a.get[3][1] * _b.get[1][3],
			_a.get[0][2] * _b.get[1][0] + _a.get[1][2] * _b.get[1][1] + _a.get[2][2] * _b.get[1][2] + _a.get[3][2] * _b.get[1][3],
			_a.get[0][3] * _b.get[1][0] + _a.get[1][3] * _b.get[1][1] + _a.get[2][3] * _b.get[1][2] + _a.get[3][3] * _b.get[1][3],
		},
		{
			_a.get[0][0] * _b.get[2][0] + _a.get[1][0] * _b.get[2][1] + _a.get[2][0] * _b.get[2][2] + _a.get[3][0] * _b.get[2][3],
			_a.get[0][1] * _b.get[2][0] + _a.get[1][1] * _b.get[2][1] + _a.get[2][1] * _b.get[2][2] + _a.get[3][1] * _b.get[2][3],
			_a.get[0][2] * _b.get[2][0] + _a.get[1][2] * _b.get[2][1] + _a.get[2][2] * _b.get[2][2] + _a.get[3][2] * _b.get[2][3],
			_a.get[0][3] * _b.get[2][0] + _a.get[1][3] * _b.get[2][1] + _a.get[2][3] * _b.get[2][2] + _a.get[3][3] * _b.get[2][3],
		},
		{
			_a.get[0][0] * _b.get[3][0] + _a.get[1][0] * _b.get[3][1] + _a.get[2][0] * _b.get[3][2] + _a.get[3][0] * _b.get[3][3],
			_a.get[0][1] * _b.get[3][0] + _a.get[1][1] * _b.get[3][1] + _a.get[2][1] * _b.get[3][2] + _a.get[3][1] * _b.get[3][3],
			_a.get[0][2] * _b.get[3][0] + _a.get[1][2] * _b.get[3][1] + _a.get[2][2] * _b.get[3][2] + _a.get[3][2] * _b.get[3][3],
			_a.get[0][3] * _b.get[3][0] + _a.get[1][3] * _b.get[3][1] + _a.get[2][3] * _b.get[3][2] + _a.get[3][3] * _b.get[3][3],
		},
	}};
}

double dot3(vec3 _a, vec3 _b) {
	return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z;
}

vec3 vabs3(vec3 _v) {
	return (vec3) {fabs(_v.x), fabs(_v.y), fabs(_v.z)};
}

vec3 cross3(vec3 _a, vec3 _b) {
	return (vec3) {_a.y * _b.z - _a.z * _b.y, _a.x * _b.z - _a.z * _b.x, _a.x * _b.y - _a.y * _b.x};
}

vec3 norm3(vec3 _v) {
	double mag = sqrt(_v.x * _v.x + _v.y * _v.y + _v.z * _v.z);
	return (vec3) {_v.x / mag, _v.y / mag, _v.z / mag};
}

double dot4(vec4 _a, vec4 _b) {
	return _a.x * _b.x + _a.y * _b.y + _a.z * _b.z + _a.w * _b.w;
}

vec4 norm4(vec4 _v) {
	double mag = sqrt(_v.x * _v.x + _v.y * _v.y + _v.z * _v.z + _v.w * _v.w);
	return (vec4) {_v.x / mag, _v.y / mag, _v.z / mag, _v.w / mag};
}

#endif