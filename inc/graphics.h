#ifndef __CORDAC_GRAPHICS_H__
#define __CORDAC_GRAPHICS_H__

#include <stdio.h>
#include <pthread.h>
#include <math.h>

#include "linuxfb.h"
#include "linalg.h"
#include "cmath.h"
#include "bcl/bmap2.c"

#define BYTE uint8_t
#define WORD uint16_t
#define DWORD uint32_t
#define QWORD uint64_t

bitmap g_tx0;
bitmap g_tx1;
bitmap g_tx2;
bitmap g_sb_xp;
bitmap g_sb_xn;
bitmap g_sb_yp;
bitmap g_sb_yn;
bitmap g_sb_zp;
bitmap g_sb_zn;

bitmap textures[16];

unsigned** zbuf;

typedef struct {
	double x, y, z;
	vec3 ax, ay, az;
} camera;

typedef struct {
	double width, height;
	double xoff, yoff;
} viewport;

typedef struct {
	WORD x : 4;
	WORD y : 4;
	WORD z : 4;
	WORD occ : 4;
	BYTE u : 4;
	BYTE v : 4;
	BYTE tid;
} vertex;

typedef struct {
	vertex* vrt[3];
} tri;

mat4 make_mview(camera* _c) {
	vec3 eye = (vec3) {_c->x, _c->y, _c->z};
	
	return (mat4) {{
		{_c->ax.x, _c->ax.y, _c->ax.z, -dot3(_c->ax, eye)},
		{_c->ay.x, _c->ay.y, _c->ay.z, -dot3(_c->ay, eye)},
		{_c->az.x, _c->az.y, _c->az.z, -dot3(_c->az, eye)},
		{0, 0, 0, 1},
	}};
}

mat4 make_mview_inv(camera* _c) {
	vec3 eye = (vec3) {_c->x, -_c->y, _c->z};
	
	double a = _c->ax.x;
	double b = _c->ax.y;
	double c = _c->ax.z;
	double d = _c->ay.x;
	double e = _c->ay.y;
	double f = _c->ay.z;
	double g = _c->az.x;
	double h = _c->az.y;
	double i = _c->az.z;
	
	double x = -dot3(_c->ax, eye);
	double y = -dot3(_c->ay, eye);
	double z = -dot3(_c->az, eye);
	
	double denom = -a * (f * h - e * i) - b * (i * d - f * g) + c * (d * h - e * g);
	
	return (mat4) {{
		{(-f * h + e * i) / denom, (-i * b + c * h) / denom, (b * f - e * c) / denom, (b * f * z - i * b * y + c * h * y - e * c * z - f * h * x + e * i * x) / denom},
		{(-i * d + f * g) / denom, (i * a - c * g) / denom, (-a * f + c * d) / denom, (-a * f * z + i * a * y + c * d * z - c * g * y - i * d * x + f * g * x) / denom},
		{(d * h - e * g) / denom, (-a * h + b * g) / denom, (e * a - b * d) / denom, (-a * (h * y - e * z) - b * d * z + b * g * y + d * h * x - e * g * x) / denom},
		{0, 0, 0, (-a * f * h + e * i * a - i * d * b + b * f * g + c * d * h - e * c * g) / denom},
	}};
}

mat4 make_perspective(double _aspect_ratio, double _fov, double _near, double _far) {
	return (mat4) {{
		{1.0 / (_aspect_ratio * tan(_fov * 0.00872664625)), 0, 0, 0},
		{0, 1.0 / tan(_fov * 0.00872664625), 0, 0},
		{0, 0, (_near + _far) / (_near - _far), 2 * _near * _far / (_near - _far)},
		{0, 0, -1.0, 0},
	}};
}

mat4 make_perspective_inv(double _aspect_ratio, double _fov, double _near, double _far) {
	return (mat4) {{
		{_aspect_ratio * tan(_fov * 0.00872664625), 0, 0, 0},
		{0, tan(_fov * 0.00872664625), 0, 0},
		{0, 0, 0, -1.0},
		{0, 0, (_near - _far) / (2 * _near * _far), (_near + _far) / (2 * _near * _far)},
	}};
}






typedef struct {
	vec3 dir;
	vec3 org;
	//vec3 sign;
	int x, y;
	double pr;
} ray;

typedef struct {
	vec3 pos;
	vec3 (*get_intersection)(ray _r);
} object;

double get_hit_sphere(int R, vec3 O, ray _r) {
	double res = 0;
	vec3 Q = (vec3) {_r.org.x - O.x, _r.org.y - O.y, _r.org.z - O.z};
	double a = dot3(_r.dir, _r.dir);
	double b = 2 * dot3(Q, _r.dir);
	double c = dot3(Q, Q) - R * R;
	double del = b * b - 4 * a * c;
	if (del >= 0) res = (-b - sqrt(del)) / (2 * a);
	res *= res > 0;
	return res;
}

double get_hit_plane(vec3 n, vec3 O, ray _r) {
	vec3 Q = (vec3) {_r.org.x - O.x, _r.org.y - O.y, _r.org.z - O.z};
	double t = -dot3(Q, n) / dot3(_r.dir, n);
	vec3 hp = (vec3) {_r.org.x + t * _r.dir.x, _r.org.y + t * _r.dir.y, _r.org.z + t * _r.dir.z};
	if ((t > 0) && (hp.x > -100) & (hp.x < 100) & (hp.z > -150) & (hp.z < 50)) return t;
	else return 0;
}

rgbx32 px_it(rgbx32 _p, double _pr) {
	return (rgbx32) {_p.r * _pr, _p.g * _pr, _p.b * _pr, _p.x * _pr};
}





// (+X) - CAST RAY ONTO Y AND Z PLANEs (ANY TWO ORTHOGONAL PLANES WILL DO) (DE-PARAMETRICISE THEM), THEN RUN BEUTHJU LINE ON EACH INDEPENDENTLY (WITH +X AS CONSTANT STEP) TO DETERMINE STEP RIGHT AND STEP UP (4 POSSIBILITIES - JUST MOVE FORWARD, MOVE FOREWARD AND RIGHT, MOVE FORWARD AND UP, MOVE FORWARD AND TO THE UPPER-RIGHT)

struct block_data {
	char* name;
	BYTE visibility : 2; // 00 = not visible, 01 = opague, 10 = translucent, 11 = reflective
	BYTE is_solid : 1; // can the block be collided with
	WORD t_id[6]; // texture ids
};

struct block_data blocks[256] = {
	[0] = {"Air", 0, 0, {0, 0, 0, 0, 0, 0}}, // air
	[1] = {"Grass", 1, 1, {1, 1, 0, 2, 1, 1}}, // grass
	[2] = {"Stone", 1, 1, {3, 3, 3, 3, 3, 3}}, // stone
	[3] = {"Dirt", 1, 1, {2, 2, 2, 2, 2, 2}}, // dirt
	[4] = {"Oak Log", 1, 1, {4, 4, 4, 4, 4, 4}}, // log
	[5] = {"Oak Leaves", 1, 1, {5, 5, 5, 5, 5, 5}}, // leaves
	[6] = {"Still Water", 2, 0, {6, 6, 6, 6, 6, 6}}, // water
};

typedef struct {
	DWORD seed;
	// ...
} world;

typedef struct {
	BYTE id;
	// ...
} voxel;

typedef struct {
	voxel get[16][16][16];
	// ...
} sector;

typedef struct c {
	sector sector[8]; // list of sectors (which contain voxel data)
	struct c *xp, *xn, *zp, *zn; // pointers to neighbouring chunks
	int x, z; // x and y position of chunk (in chunks)
	// ...
} chunk;

chunk request_chunk(int _x, int _y) {
	// check if chunk is in file, and retrieve, otherwise generate new chunk
	// ...
}

void voxel_trace(ray _r) {
	// trace voxel, check is_visible
	// ...
	
}

void draw_skybox(ray _r) {
	double u, v;
	
	vec3 adir = vabs3(_r.dir);
	vec3 dir = _r.dir;
	
	bitmap* tx_tar = &g_sb_yp;
	
	if ((adir.x > adir.z) & (adir.x > adir.y)) { // x
		if (dir.x >= 0) { // +x
			tx_tar = &g_sb_xp;
			u = (-dir.z / adir.x + 1) / 2;
			v = (dir.y / adir.x + 1) / 2;
		} else { // -x
			tx_tar = &g_sb_xn;
			u = (dir.z / adir.x + 1) / 2;
			v = (dir.y / adir.x + 1) / 2;
		}
	} else if ((adir.y > adir.x) & (adir.y > adir.z)) { // y
		if (dir.y >= 0) { // +y
			tx_tar = &g_sb_yp;
			u = (dir.x / adir.y + 1) / 2;
			v = (-dir.z / adir.y + 1) / 2;
		} else { // -y
			tx_tar = &g_sb_yn;
			u = (dir.x / adir.y + 1) / 2;
			v = (dir.z / adir.y + 1) / 2;
		}
	} else if ((adir.z > adir.x) & (adir.z > adir.y)) { // z
		if (dir.z >= 0) { // +z
			tx_tar = &g_sb_zp;
			u = (dir.x / adir.z + 1) / 2;
			v = (dir.y / adir.z + 1) / 2;
		} else { // -z
			tx_tar = &g_sb_zn;
			u = (-dir.x / adir.z + 1) / 2;
			v = (dir.y / adir.z + 1) / 2;
		}
	} else {tx_tar = &g_tx0, u = 0, v = 0;}
	
	sbuf[_r.y][_r.x] = px_it(*(rgbx32*) (&tx_tar->data[(int) (v * (g_sb_xp.info.height - 1))][(int) (u * (g_sb_xp.info.width - 1))]), _r.pr);
}

int line(chunk* _chk, ray _r) {
	double x = _r.org.x;
	double y = _r.org.y;
	double z = _r.org.z;
	
	double dx = _r.dir.x;
	double dy = _r.dir.y;
	double dz = _r.dir.z;
	
	double xi = (dx > 0) - (dx < 0); // sign of dx
	double yi = (dy > 0) - (dy < 0); // sign of dy
	double zi = (dz > 0) - (dz < 0); // sign of dz
	
	// C coefficients in equation of a line:
	// Ax + By + C = D
	double C0 = y * dx - x * dy;
	double C1 = y * dz - z * dy;
	
	// used to negate the distance checks based on line octant
	int fx = (dx < 0) ^ (dy < 0);
	int fz = (dz < 0) ^ (dy < 0);
	
	double u, v;
	
	for(int i = 0; i < 100; i++) {
		// draw pixel
		//sbuf[y0][x0] = (rgbx32) {255, 255, 255, 0};
		
		if (x >= 0 && x < 16 && y >= 0 && y < 16 && z >= 0 && z < 16)
			if (_chk->sector[0].get[(int)x][(int)y][(int)z].id) {
				sbuf[_r.y][_r.x] = (rgbx32) {255, 255, 255, 0};
				return 0;
			}
		
		// if line is above the point at (x + 1, y + 1), move up, otherwise move in the other axis (x or z)
		if ((dz * (y + yi) - dy * (z + zi) > C1) ^ fz) z += zi;
		else if ((dx * (y + yi) - dy * (x + xi) < C0) ^ fx) y += yi;
		else x += xi;
	}
	draw_skybox(_r);
}


double tceil(double _d) {
	return (_d == 0) ? 1 : ceil(_d);
}

#define sign(X) ((X > 0) - (X < 0))


double intbound(double s, double ds) {
	return ((ds > 0) ? tceil(s) - s : floor(s) - s) / ds;
}

#define RD 2
#define RD_DIAMETER 2 * RD + 1

chunk* chks[RD_DIAMETER][RD_DIAMETER];

int CUR_CHK_X;
int CUR_CHK_Z;

int t(chunk* _c, ray _r) {
	int xi = sign(_r.dir.x);
	int yi = sign(_r.dir.y);
	int zi = sign(_r.dir.z);
	
	int x = (int) _r.org.x - (_r.org.x < 0);
	int y = (int) _r.org.y - (_r.org.y < 0);
	int z = (int) _r.org.z - (_r.org.z < 0);
	
	double tx_max = intbound(_r.org.x, _r.dir.x);
	double ty_max = intbound(_r.org.y, _r.dir.y);
	double tz_max = intbound(_r.org.z, _r.dir.z);
	
	double t_dx = xi / _r.dir.x;
	double t_dy = yi / _r.dir.y;
	double t_dz = zi / _r.dir.z;
	
		
	double occ = 1;
	double t_hit = 0;
	double u, v;

	int face = 0;
	
	vec3 hit = {0};
	bitmap* tx;
	
	int id_x, id_z;
	int rel_x, rel_z;
	
	
	for (int i = 0; i < 50; i++) {
		if (tz_max < ty_max && tz_max < tx_max) {
			z += zi;
			
			t_hit = tz_max;
			tz_max += t_dz;
			face = 6 + zi;
		} else if (tx_max < ty_max) {
			x += xi;
			
			t_hit = tx_max;
			tx_max += t_dx;
			face = 2 + xi;
		} else {
			y += yi;
			
			if (y >= 128 || y < 0) break;
			
			t_hit = ty_max;
			ty_max += t_dy;
			face = 3 + yi;
		}
		
		id_x = x / 16 + RD - (x < 0) + CUR_CHK_X;
		id_z = z / 16 + RD - (z < 0) + CUR_CHK_Z;
		
		if (id_x >= 0 && id_x <= 2 * RD && id_z >= 0 && id_z <= 2 * RD) {
			rel_x = (x >= 0) ? x % 16 : 15 - abs(x) % 16;
			rel_z = (z >= 0) ? z % 16 : 15 - abs(z) % 16;
			if (chks[id_x][id_z] && chks[id_x][id_z]->sector[y / 16].get[rel_x][y % 16][rel_z].id) {
				hit = (vec3) {_r.org.x + t_hit * _r.dir.x, _r.org.y + t_hit * _r.dir.y, _r.org.z + t_hit * _r.dir.z};
				
				switch (face) {
					case 1: // +x
						u = fmod(hit.z, 1);
						u *= sign(u);
						u = 1 - u;
						v = fmod(hit.y, 1);
						tx = textures + blocks[chks[id_x][id_z]->sector[y / 16].get[rel_x][y % 16][rel_z].id].t_id[0];
					break;
					case 2: // +y
						u = fabs(fmod(hit.x, 1));
						v = fabs(fmod(hit.z, 1));
						tx = textures + blocks[chks[id_x][id_z]->sector[y / 16].get[rel_x][y % 16][rel_z].id].t_id[2];
					break;
					case 3: // -x
						u = fmod(hit.z, 1);
						u *= sign(u);
						v = fmod(hit.y, 1);
						tx = textures + blocks[chks[id_x][id_z]->sector[y / 16].get[rel_x][y % 16][rel_z].id].t_id[1];
					break;
					case 4: // -y
						u = fabs(fmod(hit.x, 1));
						v = fabs(fmod(hit.z, 1));
						tx = textures + blocks[chks[id_x][id_z]->sector[y / 16].get[rel_x][y % 16][rel_z].id].t_id[3];
					break;
					case 5: // +z
						u = fmod(hit.x, 1);
						u *= sign(u);
						v = fmod(hit.y, 1);
						tx = textures + blocks[chks[id_x][id_z]->sector[y / 16].get[rel_x][y % 16][rel_z].id].t_id[4];
					break;
					case 7: // -z
						u = fmod(hit.x, 1);
						u *= sign(u);
						u = 1 - u;
						v = fmod(hit.y, 1);
						tx = textures + blocks[chks[id_x][id_z]->sector[y / 16].get[rel_x][y % 16][rel_z].id].t_id[5];
					break;
					default:
					return 0;
				}
				
				sbuf[_r.y][_r.x] = px_it(*((rgbx32*) (&(tx->data[(int) (v * 15)][(int) (u * 15)]))), occ);
				return 0;
			}
		} else {
			//draw_skybox(_r);
			//break;
		}
	}
	return -1;
}

int get_intersection_box(ray _r, vec3i _pos, voxel _v) {
	vec3 d_inv = (vec3) {1 / _r.dir.x, 1 / _r.dir.y, 1 / _r.dir.z};
	
	double tx_bounds[2], ty_bounds[2], tz_bounds[2];
	
	tx_bounds[d_inv.x <  0] = (_pos.x - _r.org.x) * d_inv.x;
	tx_bounds[d_inv.x >= 0] = (_pos.x + 1 - _r.org.x) * d_inv.x;
	
	ty_bounds[d_inv.y <  0] = (_pos.y - _r.org.y) * d_inv.y;
	ty_bounds[d_inv.y >= 0] = (_pos.y + 1 - _r.org.y) * d_inv.y;
	
	if (tx_bounds[0] > ty_bounds[1] || ty_bounds[0] > tx_bounds[1]) return 0;
	if (ty_bounds[0] > tx_bounds[0]) tx_bounds[0] = ty_bounds[0];
	if (tx_bounds[1] > ty_bounds[1]) tx_bounds[1] = ty_bounds[1];
	
	tz_bounds[d_inv.z <  0] = (_pos.z - _r.org.z) * d_inv.z;
	tz_bounds[d_inv.z >= 0] = (_pos.z + 1 - _r.org.z) * d_inv.z;
	
	if (tx_bounds[0] > tz_bounds[1] || tz_bounds[0] > tx_bounds[1]) return 0;
	if (tz_bounds[0] > tx_bounds[0]) tx_bounds[0] = tz_bounds[0];
	if (tx_bounds[1] > tz_bounds[1]) tx_bounds[1] = tz_bounds[1];
	
	return 1;
}

void cast_ray(ray _r, camera* _c) {
	//for (int i = 0; i < num_objs; i++) {
		//if (test_hit(_s.obj[i].bounding_sphere, _r)) {
			double t = get_hit_sphere(18, (vec3) {0, 0, -50}, _r);
			double t2 = get_hit_plane((vec3) {0, -1, 0}, (vec3) {0, 50, 0}, _r);
			double t3 = get_hit_sphere(15, (vec3) {20, -40, 0}, _r);
			double t4 = get_hit_sphere(18, (vec3) {2, -18, 0}, _r);
			double t5 = get_hit_sphere(18, (vec3) {2, -20, 0}, _r);
			double t6 = get_hit_sphere(18, (vec3) {2, -42, 0}, _r);
			double t7 = get_hit_sphere(18, (vec3) {2, -64, 0}, _r);
			double t8 = get_hit_sphere(18, (vec3) {2, -86, 0}, _r);
			double t9 = get_hit_sphere(18, (vec3) {2, -108, 0}, _r);
			double t10 = get_hit_sphere(18, (vec3) {2, -120, 0}, _r);
			//double t4 = get_hit_sphere(5, (vec3) {-_c->x, -_c->y, -_c->z}, _r);
			
			double u, v;
			
			vec3 hit, nh;
			
			double tm = t;
			
			if (((t2 < tm) & (t2 != 0)) | (tm == 0)) tm = t2;
			if (((t3 < tm) & (t3 != 0)) | (tm == 0)) tm = t3;
			if (((t4 < tm) & (t4 != 0)) | (tm == 0)) tm = t4;
			if (((t5 < tm) & (t5 != 0)) | (tm == 0)) tm = t5;
			if (((t6 < tm) & (t6 != 0)) | (tm == 0)) tm = t6;
			if (((t7 < tm) & (t7 != 0)) | (tm == 0)) tm = t7;
			if (((t8 < tm) & (t8 != 0)) | (tm == 0)) tm = t8;
			if (((t9 < tm) & (t9 != 0)) | (tm == 0)) tm = t9;
			if (((t10 < tm) & (t10 != 0)) | (tm == 0)) tm = t10;
			
			if (tm) { // hit
				if (t == tm) {
					hit = (vec3) {_r.org.x + t * _r.dir.x, _r.org.y + t * _r.dir.y, _r.org.z + t * _r.dir.z};
					nh = norm3((vec3) {hit.x - 0, hit.y - 0, hit.z - (-50)});
					
					double p = 2 * dot3(_r.dir, nh);
					ray rn = (ray) {(vec3) {_r.dir.x - nh.x * p, _r.dir.y - nh.y * p, _r.dir.z - nh.z * p}, hit, _r.x, _r.y, _r.pr * 0.8};
					cast_ray(rn, _c);
				} else if (t3 == tm) {
					hit = (vec3) {_r.org.x + t3 * _r.dir.x, _r.org.y + t3 * _r.dir.y, _r.org.z + t3 * _r.dir.z};
					nh = norm3((vec3) {hit.x - (20), hit.y - (-40), hit.z - 0});
					u = 1 - (1 + atan2(nh.z, nh.x) / 3.14159265358979) / 2;
					v = acos(nh.y) / 3.14159265358979;
					
					sbuf[_r.y][_r.x] = px_it(*((rgbx32*)(&g_tx0.data[(int) (v * (g_tx0.info.height - 1))][(int) (u * (g_tx0.info.width - 1))])), _r.pr);
				/*} else if (t4 == tm) {
					hit = (vec3) {_r.org.x + t4 * _r.dir.x, _r.org.y + t4 * _r.dir.y, _r.org.z + t4 * _r.dir.z};
					nh = norm3((vec3) {hit.x + _c->x, hit.y + _c->y, hit.z + _c->z});
					u = 1 - (1 + atan2(nh.z, nh.x) / 3.14159265358979) / 2;
					v = acos(nh.y) / 3.14159265358979;
					
					sbuf[_r.y][_r.x] = px_it(*((rgbx32*)(&g_tx2.data[(int) (v * (g_tx2.info.height - 1))][(int) (u * (g_tx2.info.width - 1))])), _r.pr);*/
				} else if (t2 == tm) {
					hit = (vec3) {_r.org.x + t2 * _r.dir.x, 50, _r.org.z + t2 * _r.dir.z};
					u = dabs(hit.x) / 100;
					v = dabs(hit.z) / 100;
					u = u - (int) u;
					v = v - (int) v;
					
					sbuf[_r.y][_r.x] = px_it(*((rgbx32*)(&g_tx1.data[(int) (v * (g_tx1.info.height - 1))][(int) (u * (g_tx1.info.width - 1))])), _r.pr);
				} else if (t4 == tm) {
					hit = (vec3) {_r.org.x + t4 * _r.dir.x, _r.org.y + t4 * _r.dir.y, _r.org.z + t4 * _r.dir.z};
					nh = norm3((vec3) {hit.x - (2), hit.y - (-18), hit.z - 0});
					
					double p = 2 * dot3(_r.dir, nh);
					ray rn = (ray) {(vec3) {_r.dir.x - nh.x * p, _r.dir.y - nh.y * p, _r.dir.z - nh.z * p}, hit, _r.x, _r.y, _r.pr * 0.8};
					cast_ray(rn, _c);
				} else if (t5 == tm) {
					hit = (vec3) {_r.org.x + t5 * _r.dir.x, _r.org.y + t5 * _r.dir.y, _r.org.z + t5 * _r.dir.z};
					nh = norm3((vec3) {hit.x - (2), hit.y - (-20), hit.z - 0});
					
					double p = 2 * dot3(_r.dir, nh);
					ray rn = (ray) {(vec3) {_r.dir.x - nh.x * p, _r.dir.y - nh.y * p, _r.dir.z - nh.z * p}, hit, _r.x, _r.y, _r.pr * 0.8};
					cast_ray(rn, _c);
				} else if (t6 == tm) {
					hit = (vec3) {_r.org.x + t6 * _r.dir.x, _r.org.y + t6 * _r.dir.y, _r.org.z + t6 * _r.dir.z};
					nh = norm3((vec3) {hit.x - (2), hit.y - (-42), hit.z - 0});
					
					double p = 2 * dot3(_r.dir, nh);
					ray rn = (ray) {(vec3) {_r.dir.x - nh.x * p, _r.dir.y - nh.y * p, _r.dir.z - nh.z * p}, hit, _r.x, _r.y, _r.pr * 0.8};
					cast_ray(rn, _c);
				} else if (t7 == tm) {
					hit = (vec3) {_r.org.x + t7 * _r.dir.x, _r.org.y + t7 * _r.dir.y, _r.org.z + t7 * _r.dir.z};
					nh = norm3((vec3) {hit.x - (2), hit.y - (-64), hit.z - 0});
					
					double p = 2 * dot3(_r.dir, nh);
					ray rn = (ray) {(vec3) {_r.dir.x - nh.x * p, _r.dir.y - nh.y * p, _r.dir.z - nh.z * p}, hit, _r.x, _r.y, _r.pr * 0.8};
					cast_ray(rn, _c);
				} else if (t8 == tm) {
					hit = (vec3) {_r.org.x + t8 * _r.dir.x, _r.org.y + t8 * _r.dir.y, _r.org.z + t8 * _r.dir.z};
					nh = norm3((vec3) {hit.x - (2), hit.y - (-86), hit.z - 0});
					
					double p = 2 * dot3(_r.dir, nh);
					ray rn = (ray) {(vec3) {_r.dir.x - nh.x * p, _r.dir.y - nh.y * p, _r.dir.z - nh.z * p}, hit, _r.x, _r.y, _r.pr * 0.8};
					cast_ray(rn, _c);
				} else if (t9 == tm) {
					hit = (vec3) {_r.org.x + t9 * _r.dir.x, _r.org.y + t9 * _r.dir.y, _r.org.z + t9 * _r.dir.z};
					nh = norm3((vec3) {hit.x - (2), hit.y - (-108), hit.z - 0});
					
					double p = 2 * dot3(_r.dir, nh);
					ray rn = (ray) {(vec3) {_r.dir.x - nh.x * p, _r.dir.y - nh.y * p, _r.dir.z - nh.z * p}, hit, _r.x, _r.y, _r.pr * 0.8};
					cast_ray(rn, _c);
				} else if (t10 == tm) {
					hit = (vec3) {_r.org.x + t10 * _r.dir.x, _r.org.y + t10 * _r.dir.y, _r.org.z + t10 * _r.dir.z};
					nh = norm3((vec3) {hit.x - (2), hit.y - (-120), hit.z - 0});
					
					double p = 2 * dot3(_r.dir, nh);
					ray rn = (ray) {(vec3) {_r.dir.x - nh.x * p, _r.dir.y - nh.y * p, _r.dir.z - nh.z * p}, hit, _r.x, _r.y, _r.pr * 0.8};
					cast_ray(rn, _c);
				}
			} else { // hit nothing - draw skybox
				vec3 adir = vabs3(_r.dir);
				vec3 dir = _r.dir;
				
				bitmap* tx_tar = &g_sb_yp;
				
				if ((adir.x > adir.z) & (adir.x > adir.y)) { // x
					if (dir.x >= 0) { // +x
						tx_tar = &g_sb_xp;
						u = (dir.z / adir.x + 1) / 2;
						v = (-dir.y / adir.x + 1) / 2;
					} else { // -x
						tx_tar = &g_sb_xn;
						u = (-dir.z / adir.x + 1) / 2;
						v = (-dir.y / adir.x + 1) / 2;
					}
				} else if ((adir.y > adir.x) & (adir.y > adir.z)) { // y
					if (dir.y >= 0) { // +y
						tx_tar = &g_sb_yp;
						u = (dir.x / adir.y + 1) / 2;
						v = (-dir.z / adir.y + 1) / 2;
					} else { // -y
						tx_tar = &g_sb_yn;
						u = (dir.x / adir.y + 1) / 2;
						v = (dir.z / adir.y + 1) / 2;
					}
				} else if ((adir.z > adir.x) & (adir.z > adir.y)) { // z
					if (dir.z >= 0) { // +z
						tx_tar = &g_sb_zp;
						u = (-dir.x / adir.z + 1) / 2;
						v = (-dir.y / adir.z + 1) / 2;
					} else { // -z
						tx_tar = &g_sb_zn;
						u = (dir.x / adir.z + 1) / 2;
						v = (-dir.y / adir.z + 1) / 2;
					}
				}
				
				sbuf[_r.y][_r.x] = px_it(*(rgbx32*) (&tx_tar->data[(int) (v * (g_sb_xp.info.height - 1))][(int) (u * (g_sb_xp.info.width - 1))]), _r.pr);
			}
		//}
	//}
}

void draw(mat4 mv_inv, camera* _c, viewport* _v, chunk* chk) {
	double asp = 8/6;
	double mfov = tan(90 * 0.00872664625);
	for (int y = _v->yoff; y < _v->height + _v->yoff; y++)
	for (int x = _v->xoff; x < _v->width + _v->xoff; x++) {
		vec4 ray_dir = (vec4) {(2 * (x + 0.5) / fb_vinfo.xres - 1) * asp * mfov, (2 * (y + 0.5) / fb_vinfo.yres - 1) * mfov, -1, 1};
		vec4 ray_org = (vec4) {0, 0, 0, 1};
		ray_dir = mmul_vec4(mv_inv, ray_dir);
		ray_org = mmul_vec4(mv_inv, ray_org);
		ray r = (ray) {
			norm3((vec3) {ray_dir.x + _c->x, ray_dir.y - _c->y, ray_dir.z + _c->z}),
			(vec3) {ray_org.x, ray_org.y, ray_org.z},
			//(vec3i) {sign(ray_dir.x), sign(ray_dir.y), sign(ray_dir.z)},
			x, y, 1
		};
		if (t(chk, r) == -1) draw_skybox(r);
	}
}



















int in_tri(int x, int y, int x0, int y0, int x1, int y1, int x2, int y2) {
	return
	((x - x0) * (y1 - y0) - (y - y0) * (x1 - x0) >= 0) &
	((x - x1) * (y2 - y1) - (y - y1) * (x2 - x1) >= 0) &
	((x - x2) * (y0 - y2) - (y - y2) * (x0 - x2) >= 0);
}

void raster_tri(tri _t, viewport* _v, mat4 _mv, mat4 _mp) {
	// convert to camera space
	vec4 v0 = mmul_vec4(_mv, (vec4) {_t.vrt[0]->x, _t.vrt[0]->y, _t.vrt[0]->z, 1});
	vec4 v1 = mmul_vec4(_mv, (vec4) {_t.vrt[1]->x, _t.vrt[1]->y, _t.vrt[1]->z, 1});
	vec4 v2 = mmul_vec4(_mv, (vec4) {_t.vrt[2]->x, _t.vrt[2]->y, _t.vrt[2]->z, 1});
	
	// convert to clip space
	vec4 n0 = mmul_vec4(_mp, v0);
	vec4 n1 = mmul_vec4(_mp, v1);
	vec4 n2 = mmul_vec4(_mp, v2);
	
	// return if tri is entirely outside frustrum
	int v0cx = (n0.x < -n0.w) ? -1 : (n0.x > n0.w) ? 1 : 0;
	int v0cy = (n0.y < -n0.w) ? -1 : (n0.y > n0.w) ? 1 : 0;
	int v0cz = (n0.z < -n0.w) ? -1 : (n0.z > n0.w) ? 1 : 0;
	int v1cx = (n1.x < -n1.w) ? -1 : (n1.x > n1.w) ? 1 : 0;
	int v1cy = (n1.y < -n1.w) ? -1 : (n1.y > n1.w) ? 1 : 0;
	int v1cz = (n1.z < -n1.w) ? -1 : (n1.z > n1.w) ? 1 : 0;
	int v2cx = (n2.x < -n2.w) ? -1 : (n2.x > n2.w) ? 1 : 0;
	int v2cy = (n2.y < -n2.w) ? -1 : (n2.y > n2.w) ? 1 : 0;
	int v2cz = (n2.z < -n2.w) ? -1 : (n2.z > n2.w) ? 1 : 0;
	
	char g = 0;
	
	if ((v0cx || v0cy || v0cz) && (v1cx || v1cy || v1cz) && (v2cx || v2cy || v2cz)) g = 1;
	
	// clip against z-near plane
	
	
	// convert clip space coordinates to NDC and then screen coordinates
	vec3 c0 = (vec3) {n0.x / n0.w * (_v->width / 2) + (_v->width / 2) + _v->xoff, n0.y / n0.w * (_v->height / 2) + (_v->height / 2) + _v->yoff, n0.z / n0.w};
	vec3 c1 = (vec3) {n1.x / n1.w * (_v->width / 2) + (_v->width / 2) + _v->xoff, n1.y / n1.w * (_v->height / 2) + (_v->height / 2) + _v->yoff, n1.z / n1.w};
	vec3 c2 = (vec3) {n2.x / n2.w * (_v->width / 2) + (_v->width / 2) + _v->xoff, n2.y / n2.w * (_v->height / 2) + (_v->height / 2) + _v->yoff, n2.z / n2.w};
	
	// compute bounding box for tri
	int vp_max_x = _v->width + _v->xoff - 1;
	int vp_max_y = _v->height + _v->yoff - 1;
	
	int x0 = (int) max(_v->xoff, min3(c0.x, c1.x, c2.x));
	int y0 = (int) max(_v->yoff, min3(c0.y, c1.y, c2.y));
	int x1 = (int) min(vp_max_x, max3(c0.x, c1.x, c2.x));
	int y1 = (int) min(vp_max_y, max3(c0.y, c1.y, c2.y));
	
	// raster tri
	double A = c0.x * (c1.y - c2.y) + c1.x * (c2.y - c0.y) + c2.x * (c0.y - c1.y);
	double u = (x0 * (c1.y - c2.y) + c1.x * (c2.y - y0) + c2.x * (y0 - c1.y)) / A;
	double v = (x0 * (c2.y - c0.y) + c2.x * (c0.y - y0) + c0.x * (y0 - c2.y)) / A;
	double w = (x0 * (c0.y - c1.y) + c0.x * (c1.y - y0) + c1.x * (y0 - c0.y)) / A;
	double z, xo, yo;
	if (!g)
	for (int y = y0; y < y1; y++)
		for (int x = x0; x < x1; x++)
			if (in_tri(x, y, c0.x, c0.y, c1.x, c1.y, c2.x, c2.y)) {
				u = (x * (c1.y - c2.y) + c1.x * (c2.y - y) + c2.x * (y - c1.y)) / A;
				v = (x * (c2.y - c0.y) + c2.x * (c0.y - y) + c0.x * (y - c2.y)) / A;
				w = (x * (c0.y - c1.y) + c0.x * (c1.y - y) + c1.x * (y - c0.y)) / A;
				z = u * v0.z + v * v1.z + w * v2.z;
				if (z < zbuf[vp_max_y - y][x]) {
					sbuf[vp_max_y - y][x] = (rgbx32) {u * 255, v * 255, w * 255, 255};
					zbuf[vp_max_y - y][x] = z;
				}
			}
	/************************************************************/
	fb_swap();
	printf("bb: (%d, %d), (%d, %d)\n", x0, y0, x1, y1);
	printf("n0: %f, %f, %f\n", n0.x / n0.w, n0.y / n0.w, n0.z / n0.w);
	printf("n1: %f, %f, %f\n", n1.x / n1.w, n1.y / n1.w, n1.z / n1.w);
	printf("n1: %f, %f, %f\n", n2.x / n2.w, n2.y / n2.w, n2.z / n2.w);
	printf("c0: %f, %f, %f\n", c0.x, c0.y, c0.z);
	printf("c1: %f, %f, %f\n", c1.x, c1.y, c1.z);
	printf("c1: %f, %f, %f\n", c2.x, c2.y, c2.z);
	/***********************************************************/
}

void rotate_camera(camera* _c, double _x, double _y) {
	double cx = cos(_x);
	double cy = cos(_y);
	double sx = sin(_x);
	double sy = sin(_y);
	
	_c->ax = (vec3) {cy, 0, -sy};
	_c->ay = (vec3) {-sy * sx, -cx, -cy * sx};
	_c->az = (vec3) {sy * cx, -sx, cy * cx};
}

#endif