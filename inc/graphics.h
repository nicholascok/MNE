#ifndef __MCN_GRAPHICS_H__
#define __MCN_GRAPHICS_H__

#include <stdint.h>

#include "linuxfb.h"
#include "linalg.h"
#include "cmath.h"
#include "bcl/bmap2.c"

#define EDGE(X, Y, X0, Y0, X1, Y1) ((X - X0) * (Y1 - Y0) - (Y - Y0) * (X1 - X0))

#define IN_TRI(X, Y, V0, V1, V2, V3) (	(EDGE(X, Y, V0.x, V0.y, V1.x, V1.y) >= 0) & \
										(EDGE(X, Y, V1.x, V1.y, V2.x, V2.y) >= 0) & \
										(EDGE(X, Y, V2.x, V2.y, V0.x, V0.y) >= 0) & )

#define IN_QUAD(X, Y, V0, V1, V2, V3) (	(EDGE(X, Y, V0.x, V0.y, V1.x, V1.y) >= 0) & \
										(EDGE(X, Y, V1.x, V1.y, V2.x, V2.y) >= 0) & \
										(EDGE(X, Y, V2.x, V2.y, V3.x, V3.y) >= 0) & \
										(EDGE(X, Y, V3.x, V3.y, V0.x, V0.y) >= 0)	)

typedef struct {
	int width;
	int height;
	int xoff;
	int yoff;
} viewport;

typedef struct {
	double x;
	double y;
	double z;
	double rx;
	double ry;
	vec3 ax;
	vec3 ay;
	vec3 az;
	vec3 velocity;
	vec3 acceleration;
	double height;
	double width;
} player;

typedef struct {
	double x;
	double y;
	double z;
	BYTE occ : 2;
} vertex;

typedef struct {
	vertex* vrt[3];
	vec2i tex_coords[3];
	bitmap* tex;
	vec3 norm;
} tri;

typedef struct {
	vertex vrt[4];
	vec2i tex_coords[4];
	bitmap* tex;
	vec3 norm;
} quad;

typedef struct {
	bitmap* face[6];
} skybox;

typedef struct {
	quad* get;
	int capacity;
	int front;
	int rear;
	int len;
} quad_buffer;

int init_qbuf(quad_buffer* _qb, int capacity) {
	*_qb = (quad_buffer) {calloc(1, capacity * sizeof(quad)), capacity, 0, 0, 0};
	if (!_qb->get) return -1;
	return 0;
}

int enqueue_quad(quad_buffer* _qb, quad _q) {
	if (_qb->len + 1 >= _qb->capacity) return -1;
	_qb->len++;
	_qb->get[_qb->rear++] = _q;
	_qb->rear %= _qb->capacity;
	return 0;
};

int dequeue_quad(quad_buffer* _qb, quad* _q) {
	if (_qb->len <= 0) return -1;
	_qb->len--;
	_qb->front %= _qb->capacity;
	*_q = _qb->get[_qb->front++];
	return 0;
}

float* zbuf;

void make_mview(mat4* _r, player* _c) {
	vec3 eye = (vec3) {_c->x, _c->y, _c->z};
	
	*_r = (mat4) {{
		_c->ax.x, _c->ax.y, _c->ax.z, -dot3(_c->ax, eye),
		_c->ay.x, _c->ay.y, _c->ay.z, -dot3(_c->ay, eye),
		_c->az.x, _c->az.y, _c->az.z, -dot3(_c->az, eye),
		0, 0, 0, 1,
	}};
}

void make_perspective(mat4* _r, double _aspect_ratio, double _fov, double _near, double _far) {
	*_r = (mat4) {{
		1.0 / (_aspect_ratio * _fov), 0, 0, 0,
		0, 1.0 / _fov, 0, 0,
		0, 0, (_near + _far) / (_near - _far), 2 * _near * _far / (_near - _far),
		0, 0, -1.0, 0,
	}};
}

int clip_tri(vec4 _v0, vec4 _v1, vec4 _v2) {
	if (_v0.z < -_v0.w || _v1.z < -_v1.w || _v2.z < -_v2.w)
		return 1;
	if (_v0.x > _v0.w && _v1.x > _v1.w && _v2.x > _v2.w)
		return 1;
	if (_v0.x < -_v0.w && _v1.x < -_v1.w && _v2.x < -_v2.w)
		return 1;
	if (_v0.y > _v0.w && _v1.y > _v1.w && _v2.y > _v2.w)
		return 1;
	if (_v0.y < -_v0.w && _v1.y < -_v1.w && _v2.y < -_v2.w)
		return 1;
	
	return 0;
}

int raster_tri(tri _t, mat4* _m, viewport* _v, player* _c) {
	if (dot3((vec3) {_t.vrt[0]->x - _c->x, _t.vrt[0]->y - _c->y, _t.vrt[0]->z - _c->z}, _t.norm) < 0) return 1;
	
	vec4 v0 = (vec4) {_t.vrt[0]->x, _t.vrt[0]->y, _t.vrt[0]->z, 1.0};
	vec4 v1 = (vec4) {_t.vrt[1]->x, _t.vrt[1]->y, _t.vrt[1]->z, 1.0};
	vec4 v2 = (vec4) {_t.vrt[2]->x, _t.vrt[2]->y, _t.vrt[2]->z, 1.0};
	
	vec4 c0 = mmul_vec4(_m, v0);
	vec4 c1 = mmul_vec4(_m, v1);
	vec4 c2 = mmul_vec4(_m, v2);
	
	if (clip_tri(c0, c1, c2)) return 1;
	
	vec3 n0 = (vec3) {c0.x / c0.w, c0.y / c0.w, c0.w};
	vec3 n1 = (vec3) {c1.x / c1.w, c1.y / c1.w, c1.w};
	vec3 n2 = (vec3) {c2.x / c2.w, c2.y / c2.w, c2.w};
	
	vec2i s0 = (vec2i) {n0.x * (_v->width / 2) + (_v->width / 2) + _v->xoff, n0.y * (_v->height / 2) + (_v->height / 2) + _v->yoff};
	vec2i s1 = (vec2i) {n1.x * (_v->width / 2) + (_v->width / 2) + _v->xoff, n1.y * (_v->height / 2) + (_v->height / 2) + _v->yoff};
	vec2i s2 = (vec2i) {n2.x * (_v->width / 2) + (_v->width / 2) + _v->xoff, n2.y * (_v->height / 2) + (_v->height / 2) + _v->yoff};
	
	int bb0x = max(0, min3(s0.x, s1.x, s2.x));
	int bb0y = max(0, min3(s0.y, s1.y, s2.y));
	int bb1x = min(_v->width + _v->xoff - 1, max3(s0.x, s1.x, s2.x));
	int bb1y = min(_v->height + _v->yoff - 1, max3(s0.y, s1.y, s2.y));
	
	c0.z = 1.0 / c0.w;
	c1.z = 1.0 / c1.w;
	c2.z = 1.0 / c2.w;
	
	double tx_x0 = _t.tex_coords[0].x * c0.z;
	double tx_x1 = _t.tex_coords[1].x * c1.z;
	double tx_x2 = _t.tex_coords[2].x * c2.z;
	
	double tx_y0 = _t.tex_coords[0].y * c0.z;
	double tx_y1 = _t.tex_coords[1].y * c1.z;
	double tx_y2 = _t.tex_coords[2].y * c2.z;
	
	double occ0 = (1.0 - (double) _t.vrt[0]->occ * 0.25) * c0.z;
	double occ1 = (1.0 - (double) _t.vrt[1]->occ * 0.25) * c1.z;
	double occ2 = (1.0 - (double) _t.vrt[2]->occ * 0.25) * c2.z;
	
	double u, v, w, z;
	double tx_x, tx_y;
	double oc;
	
	double A = 1.0 / EDGE(s0.x, s0.y, s1.x, s1.y, s2.x, s2.y);
	
	double ui = (s2.y - s1.y) * A;
	double vi = (s0.y - s2.y) * A;
	double wi = (s1.y - s0.y) * A;
	
	for (int y = bb0y; y < bb1y; y++)
	for (int x = bb0x; x < bb1x; x++) {
		u = EDGE(x, y, s1.x, s1.y, s2.x, s2.y) * A;
		v = EDGE(x, y, s2.x, s2.y, s0.x, s0.y) * A;
		w = EDGE(x, y, s0.x, s0.y, s1.x, s1.y) * A;
		if ((u >= 0) & (v >= 0) & (w >= 0)) {
			z = 1.0 / (u * c0.z + v * c1.z + w * c2.z);
			
			tx_x = u * tx_x0 + v * tx_x1 + w * tx_x2;
			tx_y = u * tx_y0 + v * tx_y1 + w * tx_y2;
			oc = u * occ0 + v * occ1 + w * occ2;
			
			tx_x *= z, tx_y *= z, oc *= z;
			
			if (z < zbuf[y * fb_vinfo.xres + x]) {
				sbuf[y][x] = *((rgbx32*) &_t.tex->data[(int) (tx_x * (_t.tex->info.height - 1))][(int) (tx_y * (_t.tex->info.width - 1))]);
				sbuf[y][x].r *= oc;
				sbuf[y][x].g *= oc;
				sbuf[y][x].b *= oc;
				zbuf[y * fb_vinfo.xres + x] = z;
			}
		}
	}
	
	return 0;
}

int raster_tri_transparent(tri _t, mat4* _m, viewport* _v, player* _c) {
	if (dot3((vec3) {_t.vrt[0]->x - _c->x, _t.vrt[0]->y - _c->y, _t.vrt[0]->z - _c->z}, _t.norm) < 0) return 1;
	
	vec4 v0 = (vec4) {_t.vrt[0]->x, _t.vrt[0]->y, _t.vrt[0]->z, 1.0};
	vec4 v1 = (vec4) {_t.vrt[1]->x, _t.vrt[1]->y, _t.vrt[1]->z, 1.0};
	vec4 v2 = (vec4) {_t.vrt[2]->x, _t.vrt[2]->y, _t.vrt[2]->z, 1.0};
	
	vec4 c0 = mmul_vec4(_m, v0);
	vec4 c1 = mmul_vec4(_m, v1);
	vec4 c2 = mmul_vec4(_m, v2);
	
	if (clip_tri(c0, c1, c2)) return 1;
	
	vec3 n0 = (vec3) {c0.x / c0.w, c0.y / c0.w, c0.w};
	vec3 n1 = (vec3) {c1.x / c1.w, c1.y / c1.w, c1.w};
	vec3 n2 = (vec3) {c2.x / c2.w, c2.y / c2.w, c2.w};
	
	vec2i s0 = (vec2i) {n0.x * (_v->width / 2) + (_v->width / 2) + _v->xoff, n0.y * (_v->height / 2) + (_v->height / 2) + _v->yoff};
	vec2i s1 = (vec2i) {n1.x * (_v->width / 2) + (_v->width / 2) + _v->xoff, n1.y * (_v->height / 2) + (_v->height / 2) + _v->yoff};
	vec2i s2 = (vec2i) {n2.x * (_v->width / 2) + (_v->width / 2) + _v->xoff, n2.y * (_v->height / 2) + (_v->height / 2) + _v->yoff};
	
	int bb0x = max(0, min3(s0.x, s1.x, s2.x));
	int bb0y = max(0, min3(s0.y, s1.y, s2.y));
	int bb1x = min(_v->width + _v->xoff - 1, max3(s0.x, s1.x, s2.x));
	int bb1y = min(_v->height + _v->yoff - 1, max3(s0.y, s1.y, s2.y));
	
	c0.z = 1.0 / c0.w;
	c1.z = 1.0 / c1.w;
	c2.z = 1.0 / c2.w;
	
	double tx_x0 = _t.tex_coords[0].x * c0.z;
	double tx_x1 = _t.tex_coords[1].x * c1.z;
	double tx_x2 = _t.tex_coords[2].x * c2.z;
	
	double tx_y0 = _t.tex_coords[0].y * c0.z;
	double tx_y1 = _t.tex_coords[1].y * c1.z;
	double tx_y2 = _t.tex_coords[2].y * c2.z;
	
	double u, v, w, z;
	double tx_x, tx_y;
	
	double A = 1.0 / EDGE(s0.x, s0.y, s1.x, s1.y, s2.x, s2.y);
	
	double ui = (s2.y - s1.y) * A;
	double vi = (s0.y - s2.y) * A;
	double wi = (s1.y - s0.y) * A;
	
	rgbx32 cpx, ppx;
	
	for (int y = bb0y; y < bb1y; y++)
	for (int x = bb0x; x < bb1x; x++) {
		u = EDGE(x, y, s1.x, s1.y, s2.x, s2.y) * A;
		v = EDGE(x, y, s2.x, s2.y, s0.x, s0.y) * A;
		w = EDGE(x, y, s0.x, s0.y, s1.x, s1.y) * A;
		if ((u >= 0) & (v >= 0) & (w >= 0)) {
			z = 1.0 / (u * c0.z + v * c1.z + w * c2.z);
			
			tx_x = u * tx_x0 + v * tx_x1 + w * tx_x2;
			tx_y = u * tx_y0 + v * tx_y1 + w * tx_y2;
			
			tx_x *= z, tx_y *= z;
			
			if (z < zbuf[y * fb_vinfo.xres + x]) {
				cpx = *((rgbx32*) &_t.tex->data[(int) (tx_x * (_t.tex->info.height - 1))][(int) (tx_y * (_t.tex->info.width - 1))]);
				ppx = sbuf[y][x];
				
				sbuf[y][x] = (rgbx32) {
					(cpx.x * cpx.r + (255 - cpx.x) * ppx.r) >> 8,
					(cpx.x * cpx.g + (255 - cpx.x) * ppx.g) >> 8,
					(cpx.x * cpx.b + (255 - cpx.x) * ppx.b) >> 8,
					(cpx.x * cpx.x + (255 - cpx.x) * ppx.x) >> 8,
				};
			}
		}
	}
	
	return 0;
}

int raster_tri_alpha(tri _t, mat4* _m, viewport* _v, player* _c) {
	//if (dot3((vec3) {_t.vrt[0]->x - _c->x, _t.vrt[0]->y - _c->y, _t.vrt[0]->z - _c->z}, _t.norm) < 0) return 1;
	
	vec4 v0 = (vec4) {_t.vrt[0]->x, _t.vrt[0]->y, _t.vrt[0]->z, 1.0};
	vec4 v1 = (vec4) {_t.vrt[1]->x, _t.vrt[1]->y, _t.vrt[1]->z, 1.0};
	vec4 v2 = (vec4) {_t.vrt[2]->x, _t.vrt[2]->y, _t.vrt[2]->z, 1.0};
	
	vec4 c0 = mmul_vec4(_m, v0);
	vec4 c1 = mmul_vec4(_m, v1);
	vec4 c2 = mmul_vec4(_m, v2);
	
	if (clip_tri(c0, c1, c2)) return 1;
	
	vec3 n0 = (vec3) {c0.x / c0.w, c0.y / c0.w, c0.w};
	vec3 n1 = (vec3) {c1.x / c1.w, c1.y / c1.w, c1.w};
	vec3 n2 = (vec3) {c2.x / c2.w, c2.y / c2.w, c2.w};
	
	vec2i s0 = (vec2i) {n0.x * (_v->width / 2) + (_v->width / 2) + _v->xoff, n0.y * (_v->height / 2) + (_v->height / 2) + _v->yoff};
	vec2i s1 = (vec2i) {n1.x * (_v->width / 2) + (_v->width / 2) + _v->xoff, n1.y * (_v->height / 2) + (_v->height / 2) + _v->yoff};
	vec2i s2 = (vec2i) {n2.x * (_v->width / 2) + (_v->width / 2) + _v->xoff, n2.y * (_v->height / 2) + (_v->height / 2) + _v->yoff};
	
	int bb0x = max(0, min3(s0.x, s1.x, s2.x));
	int bb0y = max(0, min3(s0.y, s1.y, s2.y));
	int bb1x = min(_v->width + _v->xoff - 1, max3(s0.x, s1.x, s2.x));
	int bb1y = min(_v->height + _v->yoff - 1, max3(s0.y, s1.y, s2.y));
	
	c0.z = 1.0 / c0.w;
	c1.z = 1.0 / c1.w;
	c2.z = 1.0 / c2.w;
	
	double tx_x0 = _t.tex_coords[0].x * c0.z;
	double tx_x1 = _t.tex_coords[1].x * c1.z;
	double tx_x2 = _t.tex_coords[2].x * c2.z;
	
	double tx_y0 = _t.tex_coords[0].y * c0.z;
	double tx_y1 = _t.tex_coords[1].y * c1.z;
	double tx_y2 = _t.tex_coords[2].y * c2.z;
	
	double occ0 = (1.0 - (double) _t.vrt[0]->occ * 0.16666666) * c0.z;
	double occ1 = (1.0 - (double) _t.vrt[1]->occ * 0.16666666) * c1.z;
	double occ2 = (1.0 - (double) _t.vrt[2]->occ * 0.16666666) * c2.z;
	
	double u, v, w, z;
	double tx_x, tx_y;
	double oc;
	
	double A = 1.0 / EDGE(s0.x, s0.y, s1.x, s1.y, s2.x, s2.y);
	
	double ui = (s2.y - s1.y) * A;
	double vi = (s0.y - s2.y) * A;
	double wi = (s1.y - s0.y) * A;
	
	rgbx32 cpx;
	
	for (int y = bb0y; y < bb1y; y++)
	for (int x = bb0x; x < bb1x; x++) {
		u = EDGE(x, y, s1.x, s1.y, s2.x, s2.y) * A;
		v = EDGE(x, y, s2.x, s2.y, s0.x, s0.y) * A;
		w = EDGE(x, y, s0.x, s0.y, s1.x, s1.y) * A;
		if ((u >= 0) & (v >= 0) & (w >= 0)) {
			z = 1.0 / (u * c0.z + v * c1.z + w * c2.z);
			
			tx_x = u * tx_x0 + v * tx_x1 + w * tx_x2;
			tx_y = u * tx_y0 + v * tx_y1 + w * tx_y2;
			oc = u * occ0 + v * occ1 + w * occ2;
			
			tx_x *= z, tx_y *= z, oc *= z;
			
			if (z < zbuf[y * fb_vinfo.xres + x]) {
				cpx = *((rgbx32*) &_t.tex->data[(int) (tx_x * (_t.tex->info.height - 1))][(int) (tx_y * (_t.tex->info.width - 1))]);
				if (cpx.x > 127) {
					sbuf[y][x] = cpx;
					zbuf[y * fb_vinfo.xres + x] = z;
				}
			}
		}
	}
	
	return 0;
}

int raster_quad(quad _q, mat4* _m, viewport* _v, player* _c) {
	raster_tri(
		(tri) {
			{
				&_q.vrt[0],
				&_q.vrt[1],
				&_q.vrt[2],
			},
			{
				_q.tex_coords[0],
				_q.tex_coords[1],
				_q.tex_coords[2],
			},
			_q.tex, _q.norm
		}, _m, _v, _c
	);
	
	raster_tri(
		(tri) {
			{
				&_q.vrt[0],
				&_q.vrt[2],
				&_q.vrt[3],
			},
			{
				_q.tex_coords[0],
				_q.tex_coords[2],
				_q.tex_coords[3],
			},
			_q.tex, _q.norm
		}, _m, _v, _c
	);
}

int raster_quad_transparent(quad _q, mat4* _m, viewport* _v, player* _c) {
	raster_tri_transparent(
		(tri) {
			{
				&_q.vrt[0],
				&_q.vrt[1],
				&_q.vrt[2],
			},
			{
				_q.tex_coords[0],
				_q.tex_coords[1],
				_q.tex_coords[2],
			},
			_q.tex, _q.norm
		}, _m, _v, _c
	);
	
	raster_tri_transparent(
		(tri) {
			{
				&_q.vrt[0],
				&_q.vrt[2],
				&_q.vrt[3],
			},
			{
				_q.tex_coords[0],
				_q.tex_coords[2],
				_q.tex_coords[3],
			},
			_q.tex, _q.norm
		}, _m, _v, _c
	);
}

int raster_quad_alpha(quad _q, mat4* _m, viewport* _v, player* _c) {
	raster_tri_alpha(
		(tri) {
			{
				&_q.vrt[0],
				&_q.vrt[1],
				&_q.vrt[2],
			},
			{
				_q.tex_coords[0],
				_q.tex_coords[1],
				_q.tex_coords[2],
			},
			_q.tex, _q.norm
		}, _m, _v, _c
	);
	
	raster_tri_alpha(
		(tri) {
			{
				&_q.vrt[0],
				&_q.vrt[2],
				&_q.vrt[3],
			},
			{
				_q.tex_coords[0],
				_q.tex_coords[2],
				_q.tex_coords[3],
			},
			_q.tex, _q.norm
		}, _m, _v, _c
	);
}

bitmap g_sb_xp;
bitmap g_sb_xn;
bitmap g_sb_yp;
bitmap g_sb_yn;
bitmap g_sb_zp;
bitmap g_sb_zn;

rgbx32 sample_skybox(vec3 _v) {
	static bitmap* tx;
	static vec3 abs_v;
	static double u;
	static double v;
	
	abs_v = (vec3) {FABS(_v.x), FABS(_v.y), FABS(_v.z)};
	
	// determine major axis and decdie which texture to use,
	// also compute texture coordinates based on direction of vector in non-major axes.
	if ((abs_v.x > abs_v.z) & (abs_v.x > abs_v.y)) { // x
		if (_v.x >= 0) { // +x
			tx = &g_sb_xp;
			u = (-_v.z / abs_v.x + 1.0) * 0.5;
			v = ( _v.y / abs_v.x + 1.0) * 0.5;
		} else { // -x
			tx = &g_sb_xn;
			u = ( _v.z / abs_v.x + 1.0) * 0.5;
			v = ( _v.y / abs_v.x + 1.0) * 0.5;
		}
	} else if (abs_v.y > abs_v.z) { // y
		if (_v.y >= 0) { // +y
			tx = &g_sb_yp;
			u = ( _v.x / abs_v.y + 1.0) * 0.5;
			v = (-_v.z / abs_v.y + 1.0) * 0.5;
		} else { // -y
			tx = &g_sb_yn;
			u = ( _v.x / abs_v.y + 1.0) * 0.5;
			v = ( _v.z / abs_v.y + 1.0) * 0.5;
		}
	} else { // z
		if (_v.z >= 0) { // +z
			tx = &g_sb_zp;
			u = ( _v.x / abs_v.z + 1.0) * 0.5;
			v = ( _v.y / abs_v.z + 1.0) * 0.5;
		} else { // -z
			tx = &g_sb_zn;
			u = (-_v.x / abs_v.z + 1.0) * 0.5;
			v = ( _v.y / abs_v.z + 1.0) * 0.5;
		}
	}
	
	return *(rgbx32*) &tx->data[(int) (v * (tx->info.height - 1))][(int) (u * (tx->info.width - 1))];
}

int raster_skybox(mat4* _m, viewport* _v, double _fov, double _asp) {
	// compute NDC coordinates of screen corners,
	// then account for FOV and aspect ratio
	double wi = FINV((double) _v->width);
	double hi = FINV((double) _v->height);
	
	double n0x = (wi - 1.0) * _asp * _fov;
	double n0y = (hi - 1.0) * _fov;
	double n1x = (1.0 - wi) * _asp * _fov;
	double n1y = (1.0 - hi) * _fov;
	
	// generate rays through each corner of the screen.
	// (will be interpolated)
	vec3 r00 = mmul_inv_affine_no_translation(_m, (vec3) {n0x, n0y, -1.0});
	vec3 r10 = mmul_inv_affine_no_translation(_m, (vec3) {n1x, n0y, -1.0});
	vec3 r01 = mmul_inv_affine_no_translation(_m, (vec3) {n0x, n1y, -1.0});
	vec3 r11 = mmul_inv_affine_no_translation(_m, (vec3) {n1x, n1y, -1.0});
	
	// generate coefficients used for bilinear interpolation of generated rays.
	// note: this is just normal bilinear interpolation but in a form that
	// optimises operational efficiency by precomputing values, its a mess, and
	// there is no need to understand it.
	vec3 C0 = (vec3) {(double) (r00.x - r10.x - r01.x + r11.x), (double) (r00.y - r10.y - r01.y + r11.y), (double) (r00.z - r10.z - r01.z + r11.z)};
	vec3 C1 = (vec3) {(double) (r01.x - r00.x), (double) (r01.y - r00.y), (double) (r01.z - r00.z)};
	vec3 C2 = (vec3) {(double) (r10.x - r00.x), (double) (r10.y - r00.y), (double) (r10.z - r00.z)};
	
	// used to store coordinates of point in unit square.
	static double u;
	static double v;
	
	for (int y = 0; y < _v->height; y++)
	for (int x = 0; x < _v->width; x++)
	if (*((uint32_t*) &zbuf[y * fb_vinfo.xres + x]) == 0x7F7F7F7F) {
		u = (double) x * wi;
		v = (double) y * hi;
		
		sbuf[y][x] = sample_skybox((vec3) {
			v * (u * C0.x + C1.x) + u * C2.x + r00.x,
			v * (u * C0.y + C1.y) + u * C2.y + r00.y,
			v * (u * C0.z + C1.z) + u * C2.z + r00.z,
		});
	}
	
	/*******************************************************
	/* more concise, yet slower                            *
	/*******************************************************
	
	double asp = _v.width / _v.height;
	double FOV = tan(_c.FOV * 0.00872664625);
	
	double wi = 1.0 / _v.width;
	double hi = 1.0 / _v.height;
	
	for (int y = 0; y < _v.height; y++)
	for (int x = 0; x < _v.width; x++)
	if (*((uint32_t*) &zbuf[y][x]) == 0x7F7F7F7F) {
		sbuf[y][x] = sample_skybox(mmul_inv_affine_no_translation(_m, norm3((vec3) {
			((double) (2 * x + 1) * wi - 1.0) * asp * FOV,
			((double) (2 * y + 1) * hi - 1.0) * FOV,
			-1
		})));
	}
	
	/*******************************************************/
}

#endif