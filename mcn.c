#include "inc/bcl/bmap2.h"
#include "inc/prng.h"
#include "inc/linuxfb.h"
#include "inc/mouse.h"
#include "inc/keyboard.h"
#include "inc/perlin.h"
#include "inc/chk_def.h"
#include "inc/graphics.h"
#include <stdio.h>

bitmap g_textures[32];

typedef struct {
	vec3 pos;
	vec3 size;
} AAbox;

int AABB_test_collision(AAbox _a, AAbox _b) {
	return	(_a.pos.x + _a.size.x >= _b.pos.x && _b.pos.x + _b.size.x >= _a.pos.x) &&
			(_a.pos.y + _a.size.y >= _b.pos.y && _b.pos.y + _b.size.y >= _a.pos.y) &&
			(_a.pos.z + _a.size.z >= _b.pos.z && _b.pos.z + _b.size.z >= _a.pos.z);
}

int raster_chunks(mat4* _m, viewport* _v, player* _c) {
	static active_block block;
	static struct active_block_list* c_ptr;
	
	static double COX;
	static double COZ;
	
	static double off_x;
	static double off_y;
	static double off_z;
	
	for (int cx = 0; cx < RD_DIAMETER; cx++)
	for (int cz = 0; cz < RD_DIAMETER; cz++) {
		COX = (chks[0][0]->x + cx - RD) * 16;
		COZ = (chks[0][0]->z + cz - RD) * 16;
		for (int i = 0; i < chk_meshes[cx][cz].len; i++) {
			block = chk_meshes[cx][cz].get[i];
			switch (blocks[block.id].visibility) {
				case 1:
					if (block.face_xp) { // +X
						raster_quad((quad) {
							{
								(vertex) {COX + block.x + 1, block.y    , COZ + block.z    , block.occ100},
								(vertex) {COX + block.x + 1, block.y    , COZ + block.z + 1, block.occ101},
								(vertex) {COX + block.x + 1, block.y + 1, COZ + block.z + 1, block.occ111},
								(vertex) {COX + block.x + 1, block.y + 1, COZ + block.z    , block.occ110},
							},
							{(vec2i) {0, 1}, (vec2i) {0, 0}, (vec2i) {1, 0}, (vec2i) {1, 1}},
							&g_textures[blocks[block.id].t_id[0]], (vec3) {-1, 0, 0}
						}, _m, _v, _c);
					}
					
					if (block.face_xn) { // -X
						raster_quad((quad) {
							{
								(vertex) {COX + block.x    , block.y + 1, COZ + block.z    , block.occ010},
								(vertex) {COX + block.x    , block.y    , COZ + block.z    , block.occ000},
								(vertex) {COX + block.x    , block.y    , COZ + block.z + 1, block.occ001},
								(vertex) {COX + block.x    , block.y + 1, COZ + block.z + 1, block.occ011},
							},
							{(vec2i) {1, 1}, (vec2i) {0, 1}, (vec2i) {0, 0}, (vec2i) {1, 0}},
							&g_textures[blocks[block.id].t_id[1]], (vec3) {1, 0, 0}
						}, _m, _v, _c);
					}
					
					if (block.face_yp) { // +Y
						raster_quad((quad) {
							{
								(vertex) {COX + block.x    , block.y + 1 - blocks[block.id].height_mod * 0.0625, COZ + block.z    , block.occ010},
								(vertex) {COX + block.x    , block.y + 1 - blocks[block.id].height_mod * 0.0625, COZ + block.z + 1, block.occ011},
								(vertex) {COX + block.x + 1, block.y + 1 - blocks[block.id].height_mod * 0.0625, COZ + block.z + 1, block.occ111},
								(vertex) {COX + block.x + 1, block.y + 1 - blocks[block.id].height_mod * 0.0625, COZ + block.z    , block.occ110},
							},
							{(vec2i) {0, 1}, (vec2i) {0, 0}, (vec2i) {1, 0}, (vec2i) {1, 1}},
							&g_textures[blocks[block.id].t_id[2]], (vec3) {0, -1, 0}
						}, _m, _v, _c);
					}
					
					if (block.face_yn) { // -Y
						raster_quad((quad) {
							{
								(vertex) {COX + block.x    , block.y    , COZ + block.z    , block.occ000},
								(vertex) {COX + block.x    , block.y    , COZ + block.z + 1, block.occ001},
								(vertex) {COX + block.x + 1, block.y    , COZ + block.z + 1, block.occ101},
								(vertex) {COX + block.x + 1, block.y    , COZ + block.z    , block.occ100},
							},
							{(vec2i) {1, 0}, (vec2i) {1, 1}, (vec2i) {0, 1}, (vec2i) {0, 0}},
							&g_textures[blocks[block.id].t_id[3]], (vec3) {0, 1, 0}
						}, _m, _v, _c);
					}
					
					if (block.face_zp) { // +Z
						raster_quad((quad) {
							{
								(vertex) {COX + block.x    , block.y    , COZ + block.z + 1, block.occ001},
								(vertex) {COX + block.x + 1, block.y    , COZ + block.z + 1, block.occ101},
								(vertex) {COX + block.x + 1, block.y + 1, COZ + block.z + 1, block.occ111},
								(vertex) {COX + block.x    , block.y + 1, COZ + block.z + 1, block.occ011},
							},
							{(vec2i) {0, 0}, (vec2i) {0, 1}, (vec2i) {1, 1}, (vec2i) {1, 0}},
							&g_textures[blocks[block.id].t_id[4]], (vec3) {0, 0, -1}
						}, _m, _v, _c);
					}
					
					if (block.face_zn) { // -Z
						raster_quad((quad) {
							{
								(vertex) {COX + block.x    , block.y    , COZ + block.z    , block.occ000},
								(vertex) {COX + block.x + 1, block.y    , COZ + block.z    , block.occ100},
								(vertex) {COX + block.x + 1, block.y + 1, COZ + block.z    , block.occ110},
								(vertex) {COX + block.x    , block.y + 1, COZ + block.z    , block.occ010},
							},
							{(vec2i) {0, 1}, (vec2i) {0, 0}, (vec2i) {1, 0}, (vec2i) {1, 1}},
							&g_textures[blocks[block.id].t_id[5]], (vec3) {0, 0, 1}
						}, _m, _v, _c);
					}
					break;
				case 3:
					if (block.face_am) { // main diagonal
						off_x = perlin(100.5, (block.x + chks[cx][cz]->x * 16) * 64, (block.z + chks[cx][cz]->z * 16) * 64) * 0.5;
						off_y = (double) IFLOOR(FABS(perlin((block.x + chks[cx][cz]->x * 16) * 64, 100.5, (block.z + chks[cx][cz]->z * 16) * 64) * 8));
						off_z = perlin((block.x + chks[cx][cz]->x * 16) * 64, (block.z + chks[cx][cz]->z * 16), 100.5) * 0.5;
						
						raster_quad_alpha((quad) {
							{
								(vertex) {COX + off_x + block.x + 0.8, block.y     - off_y * 0.0625, COZ + off_z + block.z      , 0},
								(vertex) {COX + off_x + block.x      , block.y     - off_y * 0.0625, COZ + off_z + block.z + 0.8, 0},
								(vertex) {COX + off_x + block.x      , block.y + 1 - off_y * 0.0625, COZ + off_z + block.z + 0.8, 0},
								(vertex) {COX + off_x + block.x + 0.8, block.y + 1 - off_y * 0.0625, COZ + off_z + block.z      , 0},
							},
							{(vec2i) {0, 1}, (vec2i) {0, 0}, (vec2i) {1, 0}, (vec2i) {1, 1}},
							&g_textures[blocks[block.id].t_id[0]], (vec3) {0, 0, 0}
						}, _m, _v, _c);
					}
					
					if (block.face_as) { // -X
						raster_quad_alpha((quad) {
							{
								(vertex) {COX + off_x + block.x + 0.8, block.y     - off_y * 0.0625, COZ + off_z + block.z + 0.8, 0},
								(vertex) {COX + off_x + block.x      , block.y     - off_y * 0.0625, COZ + off_z + block.z      , 0},
								(vertex) {COX + off_x + block.x      , block.y + 1 - off_y * 0.0625, COZ + off_z + block.z      , 0},
								(vertex) {COX + off_x + block.x + 0.8, block.y + 1 - off_y * 0.0625, COZ + off_z + block.z + 0.8, 0},
							},
							{(vec2i) {0, 1}, (vec2i) {0, 0}, (vec2i) {1, 0}, (vec2i) {1, 1}},
							&g_textures[blocks[block.id].t_id[0]], (vec3) {0, 0, 0}
						}, _m, _v, _c);
					}
					break;
				default:
					break;
			}
		}
	}
	
	for (int cx = 0; cx < RD_DIAMETER; cx++)
	for (int cz = 0; cz < RD_DIAMETER; cz++) {
		COX = (chks[0][0]->x + cx - RD) * 16;
		COZ = (chks[0][0]->z + cz - RD) * 16;
		for (int i = 0; i < chk_deferred_meshes[cx][cz].len; i++) {
			block = chk_deferred_meshes[cx][cz].get[i];
			
			if (block.face_xp) { // +x
				raster_quad_transparent((quad) {
					{
						(vertex) {COX + block.x + 1, block.y    , COZ + block.z    , block.occ100},
						(vertex) {COX + block.x + 1, block.y    , COZ + block.z + 1, block.occ101},
						(vertex) {COX + block.x + 1, block.y + 1, COZ + block.z + 1, block.occ111},
						(vertex) {COX + block.x + 1, block.y + 1, COZ + block.z    , block.occ110},
					},
					{(vec2i) {0, 1}, (vec2i) {0, 0}, (vec2i) {1, 0}, (vec2i) {1, 1}},
					&g_textures[blocks[block.id].t_id[0]], (vec3) {-1, 0, 0}
				}, _m, _v, _c);
			}
			
			if (block.face_xn) { // -X
				raster_quad_transparent((quad) {
					{
						(vertex) {COX + block.x    , block.y + 1, COZ + block.z    , block.occ010},
						(vertex) {COX + block.x    , block.y    , COZ + block.z    , block.occ000},
						(vertex) {COX + block.x    , block.y    , COZ + block.z + 1, block.occ001},
						(vertex) {COX + block.x    , block.y + 1, COZ + block.z + 1, block.occ011},
					},
					{(vec2i) {0, 0}, (vec2i) {0, 1}, (vec2i) {1, 1}, (vec2i) {1, 0}},
					&g_textures[blocks[block.id].t_id[1]], (vec3) {1, 0, 0}
				}, _m, _v, _c);
			}
			
			if (block.face_yp) { // +Y
				raster_quad_transparent((quad) {
					{
						(vertex) {COX + block.x    , block.y + 1 - blocks[block.id].height_mod * 0.0625, COZ + block.z    , block.occ010},
						(vertex) {COX + block.x    , block.y + 1 - blocks[block.id].height_mod * 0.0625, COZ + block.z + 1, block.occ011},
						(vertex) {COX + block.x + 1, block.y + 1 - blocks[block.id].height_mod * 0.0625, COZ + block.z + 1, block.occ111},
						(vertex) {COX + block.x + 1, block.y + 1 - blocks[block.id].height_mod * 0.0625, COZ + block.z    , block.occ110},
					},
					{(vec2i) {0, 1}, (vec2i) {0, 0}, (vec2i) {1, 0}, (vec2i) {1, 1}},
					&g_textures[blocks[block.id].t_id[2]], (vec3) {0, -1, 0}
				}, _m, _v, _c);
			}
			
			if (block.face_yn) { // -Y
				raster_quad_transparent((quad) {
					{
						(vertex) {COX + block.x    , block.y    , COZ + block.z    , block.occ000},
						(vertex) {COX + block.x    , block.y    , COZ + block.z + 1, block.occ001},
						(vertex) {COX + block.x + 1, block.y    , COZ + block.z + 1, block.occ101},
						(vertex) {COX + block.x + 1, block.y    , COZ + block.z    , block.occ100},
					},
					{(vec2i) {1, 0}, (vec2i) {1, 1}, (vec2i) {0, 1}, (vec2i) {0, 0}},
					&g_textures[blocks[block.id].t_id[3]], (vec3) {0, 1, 0}
				}, _m, _v, _c);
			}
			
			if (block.face_zp) { // +Z
				raster_quad_transparent((quad) {
					{
						(vertex) {COX + block.x    , block.y    , COZ + block.z + 1, block.occ001},
						(vertex) {COX + block.x + 1, block.y    , COZ + block.z + 1, block.occ101},
						(vertex) {COX + block.x + 1, block.y + 1, COZ + block.z + 1, block.occ111},
						(vertex) {COX + block.x    , block.y + 1, COZ + block.z + 1, block.occ011},
					},
					{(vec2i) {0, 0}, (vec2i) {0, 1}, (vec2i) {1, 1}, (vec2i) {1, 0}},
					&g_textures[blocks[block.id].t_id[4]], (vec3) {0, 0, -1}
				}, _m, _v, _c);
			}
			
			if (block.face_zn) { // -Z
				raster_quad_transparent((quad) {
					{
						(vertex) {COX + block.x    , block.y    , COZ + block.z    , block.occ000},
						(vertex) {COX + block.x + 1, block.y    , COZ + block.z    , block.occ100},
						(vertex) {COX + block.x + 1, block.y + 1, COZ + block.z    , block.occ110},
						(vertex) {COX + block.x    , block.y + 1, COZ + block.z    , block.occ010},
					},
					{(vec2i) {0, 1}, (vec2i) {0, 0}, (vec2i) {1, 0}, (vec2i) {1, 1}},
					&g_textures[blocks[block.id].t_id[5]], (vec3) {0, 0, 1}
				}, _m, _v, _c);
			}
		}
	}
	
	return 0;
}

int update_client(player* _c, double _dt) {
	_c->x += _c->velocity.x * _dt;
	_c->y += _c->velocity.y * _dt;
	_c->z += _c->velocity.z * _dt;
	
}

int client_update_rotation(player* _c) {
	static mouse_event me;
	me = get_mouse_event();

	_c->ry -= me.rel_x * 0.0078125;
	_c->rx += me.rel_y * 0.0078125;

	if (_c->rx < -1.57079632679) _c->rx = -1.57079632679;
	if (_c->rx > 1.57079632679) _c->rx = 1.57079632679;
	if (_c->ry > 3.14159265359) _c->ry -= 6.28318530718;
	if (_c->ry <= -3.14159265359) _c->ry += 6.28318530718;

	double cx = cos(_c->rx);
	double cy = cos(_c->ry);
	double sx = sin(_c->rx);
	double sy = sin(_c->ry);

	_c->ax = (vec3) {cy, 0, -sy};
	_c->ay = (vec3) {-sy * sx, -cx, -cy * sx};
	_c->az = (vec3) {sy * cx, -sx, cy * cx};

	return 0;
}

int client_update_velocity_from_keystate(player* _c, double _speed) {
	static uint16_t kbuf[16] = {0};

	sync_keymap();
	add_keys_to_buffer(kbuf, 16);

	struct {
		BYTE kf : 1; // forward
		BYTE kb : 1; // backward
		BYTE kl : 1; // left
		BYTE kr : 1; // right
		BYTE ku : 1; // up
		BYTE kd : 1; // down
	} kstat;

	kstat.kf = check_key_local(KEY_W);
	kstat.kb = check_key_local(KEY_S);
	kstat.kl = check_key_local(KEY_A);
	kstat.kr = check_key_local(KEY_D);
	kstat.ku = check_key_local(KEY_SPACE);
	kstat.kd = check_key_local(KEY_LEFTSHIFT);

	switch(*((BYTE*) &kstat) & 0x0F) {
		case 0x01: case 0x0D:	_c->velocity = (vec3) {-_c->az.x				, 0, -_c->az.z				  }; break;
		case 0x02: case 0x0E:	_c->velocity = (vec3) { _c->az.x				, 0,  _c->az.z				  }; break;
		case 0x04: case 0x07:	_c->velocity = (vec3) {-_c->ax.x				, 0, -_c->ax.z				  }; break;
		case 0x08: case 0x0B:	_c->velocity = (vec3) { _c->ax.x				, 0,  _c->ax.z				  }; break;
		case 0x05:				_c->velocity = (vec3) {-_c->ax.x -	2 * _c->az.x, 0, -_c->ax.z -  2 * _c->az.z}; break;
		case 0x06:				_c->velocity = (vec3) {-_c->ax.x +	2 * _c->az.x, 0, -_c->ax.z +  2 * _c->az.z}; break;
		case 0x09:				_c->velocity = (vec3) { _c->ax.x -	2 * _c->az.x, 0,  _c->ax.z -  2 * _c->az.z}; break;
		case 0x0A:				_c->velocity = (vec3) { _c->ax.x +	2 * _c->az.x, 0,  _c->ax.z +  2 * _c->az.z}; break;
		default:				_c->velocity = (vec3) { 0						, 0,  0						  }; break;
	}

	_c->velocity = norm3(_c->velocity);
	_c->velocity.x *= _speed;
	_c->velocity.z *= _speed;

	switch (*((BYTE*) &kstat) & 0xF0) {
		case 0x10: _c->velocity.y =  _speed; break;
		case 0x20: _c->velocity.y = -_speed; break;
		default:   _c->velocity.y =  0     ; break;
	}

	return 0;
}

int main(void) {
	// init framebuffer
	switch (fb_init("/dev/fb0")) {
		case 1:	 printf("error: failed to open framebuffer device at /dev/fb0.\n"); return 1;
		case 2:	 printf("error: failed to retreive variable or fixed screen information.\n"); return 1;
		case 3:	 printf("error: failed to update variable screen information (FBIOPAN_DISPLAY).\n"); return 1;
		case 4:	 printf("error: 32-bit colour depth must be supported by the system.\n"); return 1;
		case 5:	 printf("error: non standard pixel format not supported.\n"); return 1;
		case 6:	 printf("error: failed to map framebuffer to memory.\n"); return 1;
		default: printf("error: failed to initialise framebuffer.\n"); return 1;
		case 0: break;
	}

	// init mouse and keyboard
	if (mouse_init("/dev/input/mice")) printf("error: failed to open mouse device");
	if (keyboard_init("/dev/input/event2", O_NONBLOCK)) printf("error: failed to open keyboard device");

	// init perlin noise
	pl_init(669421, 4, 1, 0.6);

	// init depth buffer
	zbuf = calloc(fb_vinfo.xres * fb_vinfo.yres, sizeof(float));

	// init client
	viewport v = {800, 600, 0, 0};

	player client = {7.5, 64.0, 7.5, 0, 3.1415926535898, (vec3) {1.0, 0.0, 0.0}, (vec3) {0.0, 1.0, 0.0}, (vec3) {0.0, 0.0, 1.0}, (vec3) {0.0, 0.0, 0.0}, (vec3) {0.0, 0.0, 0.0}, 1.8125, 0.6};
	client_config.FOV = tan(90 * 0.00872664625);
	client_config.ASPECT_RATIO = (double) v.width / (double) v.height;

	double WALK_SPEED = 6.0;
	double RUN_SPEED = 9.0;

	// load textures
	g_sb_xp = import_bitmap("xp.bmp");
	g_sb_xn = import_bitmap("xn.bmp");
	g_sb_yp = import_bitmap("yp.bmp");
	g_sb_yn = import_bitmap("yn.bmp");
	g_sb_zp = import_bitmap("zp.bmp");
	g_sb_zn = import_bitmap("zn.bmp");

	g_textures[0 ] = import_bitmap("void.bmp");
	g_textures[1 ] = import_bitmap("grass_top.bmp");
	g_textures[2 ] = import_bitmap("grass_side.bmp");
	g_textures[3 ] = import_bitmap("grass_bottom.bmp");
	g_textures[4 ] = import_bitmap("stone.bmp");
	g_textures[5 ] = import_bitmap("log.bmp");
	g_textures[6 ] = import_bitmap("leaves.bmp");
	g_textures[7 ] = import_bitmap("water_still.bmp");
	g_textures[8 ] = import_bitmap("sand.bmp");
	g_textures[9 ] = import_bitmap("gravel.bmp");
	g_textures[10] = import_bitmap("wild_grass.bmp");

	// fetch initial chunks
	printf("generating chunks\n");

	CHK_X = 0, CHK_Z = 0;
	int OLD_CHK_X = CHK_X, OLD_CHK_Z = CHK_Z;

	for (int i = 0; i < RD_DIAMETER; i++)
	for (int j = 0; j < RD_DIAMETER; j++) {
		printf("generating chunk at CX %i, CZ %i\n", i, j);
		chks[i][j] = fetch_chunk(i + CHK_X, j + CHK_Z);
	}
	
	init_chunks();
	
	printf("chunks generated\n");

	// variables to store mouse and key events
	key_event ke;

	// setup matrices (world-to-camera and perspective)
	mat4 m_proj;
	make_perspective(&m_proj, client_config.ASPECT_RATIO, client_config.FOV, 0.1, 100);
	mat4 m_mview;
	mat4 m_all;

	for (;;) {
		// clear depth buffer
		memset(zbuf, 0x7F, fb_vinfo.xres * fb_vinfo.yres * sizeof(float));

		// compute player movement
		ke = get_key_event_no_buffer();

		//if (ke.key == 'i') WALK_SPEED += 0.5;
		//if (ke.key == 'o') WALK_SPEED -= 0.5;

		client_update_velocity_from_keystate(&client, WALK_SPEED);

		int id = update_client(&client, 0.1);

		// handle mouse movements
		client_update_rotation(&client);

		// fetch chunks
		update_chunks(client.x, client.z);

		// draw
		make_mview(&m_mview, &client);
		mmul4(&m_all, &m_mview, &m_proj);

		raster_chunks(&m_all, &v, &client);
		raster_skybox(&m_mview, &v, client_config.FOV, client_config.ASPECT_RATIO);

		fb_copy();

		// debug
		//printf("\e[0;0Hrot: x %f, y %f \npos: x %f, y %f, z %f \nvel: x %f, y %f, z %f \nchk: x %i, z %i \nHIT: -\n", client.rx / 3.1415926535 * 180.0, client.ry / 3.1415926535 * 180.0, client.x, client.y, client.z, client.velocity.x, client.velocity.y, client.velocity.z, CHK_X, CHK_Z);
	}

	return 0;
}