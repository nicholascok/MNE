#include "inc/bcl/bmap2.h"
#include "inc/graphics.h"
#include "inc/perlin.h"
#include "inc/prng.h"
#include "inc/linuxfb.h"
#include "inc/mouse.h"
#include "inc/chk_def.h"

#include <stdio.h>
#include <termios.h>

bitmap g_textures[32];

char get_char() {
		char buf[16] = {0};
		
		// set terminal flags and enter raw mode
        struct termios old = {0};
        tcgetattr(0, &old);
        old.c_lflag &= ~ICANON;
        old.c_lflag |= ECHO;
        old.c_cc[VMIN] = 0;
        old.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &old);
		
        read(0, &buf, 16);
		
		// restore flags and re-enter cooked mode
        old.c_lflag |= ICANON;
        old.c_lflag &= ~ECHO;
        tcsetattr(0, TCSADRAIN, &old);

		
        return buf[0];
}

int update_chunks(camera _c) {
		CHK_X = CREM16(_c.x); 
		CHK_Z = CREM16(_c.z);
		
		if (CHK_X > OLD_CHK_X) {
			for (int i = 0; i < RD_DIAMETER - 1; i++)
				for (int j = 0; j < RD_DIAMETER; j++)
					loaded_chunks[i][j] = loaded_chunks[i + 1][j];
			for (int i = 0; i < RD_DIAMETER; i++)
				loaded_chunks[RD_DIAMETER - 1][i] = fetch_chunk(CHK_X + RD_DIAMETER - 1, OLD_CHK_Z + i);
			OLD_CHK_X = CHK_X;
		} else if (CHK_X < OLD_CHK_X) {
			for (int i = RD_DIAMETER - 1; i > 0; i--)
				for (int j = 0; j < RD_DIAMETER; j++)
					loaded_chunks[i][j] = loaded_chunks[i - 1][j];
			for (int i = 0; i < RD_DIAMETER; i++)
				loaded_chunks[0][i] = fetch_chunk(CHK_X, OLD_CHK_Z + i);
			OLD_CHK_X = CHK_X;
		}
		
		if (CHK_Z > OLD_CHK_Z) {
			for (int i = 0; i < RD_DIAMETER; i++)
				for (int j = 0; j < RD_DIAMETER - 1; j++)
					loaded_chunks[i][j] = loaded_chunks[i][j + 1];
			for (int i = 0; i < RD_DIAMETER; i++)
				loaded_chunks[i][RD_DIAMETER - 1] = fetch_chunk(OLD_CHK_X + i, CHK_Z + RD_DIAMETER - 1);
			OLD_CHK_Z = CHK_Z;
		} else if (CHK_Z < OLD_CHK_Z) {
			for (int i = 0; i < RD_DIAMETER; i++)
				for (int j = RD_DIAMETER - 1; j > 0; j--)
					loaded_chunks[i][j] = loaded_chunks[i][j - 1];
			for (int i = 0; i < RD_DIAMETER; i++)
				loaded_chunks[i][0] = fetch_chunk(OLD_CHK_X + i, CHK_Z);
			OLD_CHK_Z = CHK_Z;
		}
}

int raster_chunks(mat4 _m, viewport _v, camera _c) {
	for (int x = 0; x < 16 * RD_DIAMETER; x++)
	for (int z = 0; z < 16 * RD_DIAMETER; z++)
	for (int y = 0; y < 128; y++) {
		
		int CX = x / 16;
		int CY = y / 16;
		int CZ = z / 16;
		
		int rx = x % 16;
		int ry = y % 16;
		int rz = z % 16;
		
		if (loaded_chunks[CX][CZ]->sector[CY].get[rx][ry][rz].id) {
			vertex v000 = (vertex) {(double) (x     + loaded_chunks[0][0]->x * 16 - 8 * RD), (double) (y    ), (double) (z     + loaded_chunks[0][0]->z * 16 - 8 * RD), 0};
			vertex v001 = (vertex) {(double) (x     + loaded_chunks[0][0]->x * 16 - 8 * RD), (double) (y    ), (double) (z + 1 + loaded_chunks[0][0]->z * 16 - 8 * RD), 0};
			vertex v010 = (vertex) {(double) (x     + loaded_chunks[0][0]->x * 16 - 8 * RD), (double) (y + 1), (double) (z     + loaded_chunks[0][0]->z * 16 - 8 * RD), 0};
			vertex v011 = (vertex) {(double) (x     + loaded_chunks[0][0]->x * 16 - 8 * RD), (double) (y + 1), (double) (z + 1 + loaded_chunks[0][0]->z * 16 - 8 * RD), 0};
			vertex v100 = (vertex) {(double) (x + 1 + loaded_chunks[0][0]->x * 16 - 8 * RD), (double) (y    ), (double) (z     + loaded_chunks[0][0]->z * 16 - 8 * RD), 0};
			vertex v101 = (vertex) {(double) (x + 1 + loaded_chunks[0][0]->x * 16 - 8 * RD), (double) (y    ), (double) (z + 1 + loaded_chunks[0][0]->z * 16 - 8 * RD), 0};
			vertex v110 = (vertex) {(double) (x + 1 + loaded_chunks[0][0]->x * 16 - 8 * RD), (double) (y + 1), (double) (z     + loaded_chunks[0][0]->z * 16 - 8 * RD), 0};
			vertex v111 = (vertex) {(double) (x + 1 + loaded_chunks[0][0]->x * 16 - 8 * RD), (double) (y + 1), (double) (z + 1 + loaded_chunks[0][0]->z * 16 - 8 * RD), 0};
			
			int block_id = loaded_chunks[CX][CZ]->sector[CY].get[rx][ry][rz].id;
			
			if (x < VRD && !loaded_chunks[(x + 1) / 16][CZ]->sector[CY].get[(x + 1) % 16][ry][rz].id) {				
				raster_quad((quad) {
					{v100, v101, v111, v110},
					{(vec2i) {0, 1}, (vec2i) {0, 0}, (vec2i) {1, 0}, (vec2i) {1, 1}},
					&g_textures[blocks[block_id].t_id[0]],
					(vec3) {-1, 0, 0}
				}, _m, _v, _c);
			}
			
			if (x > 0 && !loaded_chunks[(x - 1) / 16][CZ]->sector[CY].get[(x - 1) % 16][y % 16][rz].id) {			
				raster_quad((quad) {
					{v000, v001, v011, v010},
					{(vec2i) {0, 0}, (vec2i) {0, 1}, (vec2i) {1, 1}, (vec2i) {1, 0}},
					&g_textures[blocks[block_id].t_id[1]],
					(vec3) {1, 0, 0}
				}, _m, _v, _c);
			}

			if (y < 127 && !loaded_chunks[CX][CZ]->sector[(y + 1) / 16].get[rx][(y + 1) % 16][rz].id) {			
				raster_quad((quad) {
					{v010, v011, v111, v110},
					{(vec2i) {0, 1}, (vec2i) {0, 0}, (vec2i) {1, 0}, (vec2i) {1, 1}},
					&g_textures[blocks[block_id].t_id[2]],
					(vec3) {0, -1, 0}
				}, _m, _v, _c);
			}

			if (y > 0 && !loaded_chunks[CX][CZ]->sector[(y - 1) / 16].get[rx][(y - 1) % 16][rz].id) {			
				raster_quad((quad) {
					{v000, v001, v101, v100},
					{(vec2i) {1, 0}, (vec2i) {1, 1}, (vec2i) {0, 1}, (vec2i) {0, 0}},
					&g_textures[blocks[block_id].t_id[3]],
					(vec3) {0, 1, 0}
				}, _m, _v, _c);
			}

			if (z < VRD && !loaded_chunks[CX][(z + 1) / 16]->sector[CY].get[rx][ry][(z + 1) % 16].id) {
				raster_quad((quad) {
					{v001, v101, v111, v011},
					{(vec2i) {0, 0}, (vec2i) {0, 1}, (vec2i) {1, 1}, (vec2i) {1, 0}},
					&g_textures[blocks[block_id].t_id[4]],
					(vec3) {0, 0, -1}
				}, _m, _v, _c);
			}

			if (z > 0 && !loaded_chunks[CX][(z - 1) / 16]->sector[CY].get[rx][ry][(z - 1) % 16].id) {			
				raster_quad((quad) {
					{v000, v100, v110, v010},
					{(vec2i) {0, 1}, (vec2i) {0, 0}, (vec2i) {1, 0}, (vec2i) {1, 1}},
					&g_textures[blocks[block_id].t_id[5]],
					(vec3) {0, 0, 1}
				}, _m, _v, _c);
			}
		}
	}
}

int main(void) {
	if (mouse_init("/dev/input/mice")) printf("error: failed to open mouse device");
	
	pl_init(0x69696969, 3, 2, 0.4);
	pl_init(669421, 2, 1.5, 0.6);
	
	switch (fb_init("/dev/fb0")) {
		case 1: printf("error: failed to open framebuffer device at /dev/fb0."); return 1;
		case 2: printf("error: failed to retreive variable or fixed screen information."); return 1;
		case 3: printf("error: failed to update variable screen information."); return 1;
		case 4: printf("error: 32-bit colour depth must be supported by the system."); return 1;
		case 5: printf("error: non standard pixel format not supported."); return 1;
		case 6: printf("error: failed to map framebuffer to memory."); return 1;
		default: printf("error: failed to initialise framebuffer."); return 1;
		case 0: break;
	}
	
	zbuf = calloc(1, fb_vinfo.yres * sizeof(void*));
	for (int i = 0; i < fb_vinfo.yres; i++)
		zbuf[i] = calloc(1, fb_vinfo.xres * sizeof(float));
	
	g_sb_xp = import_bitmap("xp.bmp");
	g_sb_xn = import_bitmap("xn.bmp");
	g_sb_yp = import_bitmap("yp.bmp");
	g_sb_yn = import_bitmap("yn.bmp");
	g_sb_zp = import_bitmap("zp.bmp");
	g_sb_zn = import_bitmap("zn.bmp");
	
	g_textures[0] = import_bitmap("grass_top.bmp");
	g_textures[1] = import_bitmap("grass_side.bmp");
	g_textures[2] = import_bitmap("grass_bottom.bmp");
	g_textures[3] = import_bitmap("stone.bmp");
	g_textures[4] = import_bitmap("log.bmp");
	g_textures[5] = import_bitmap("leaves.bmp");
	g_textures[6] = import_bitmap("water_still.bmp");
	
	camera c = {7.5, 64, 7.5, (vec3) {1, 0, 0}, (vec3) {0, 1, 0}, (vec3) {0, 0, 1}, 90};
	viewport v = {800, 600, 0, 0};
	
	printf("generating chunks\n");
	
	CHK_X = 0; 
	CHK_Z = 0;
	
	int OLD_CHK_X = CHK_X;
	int OLD_CHK_Z = CHK_Z;
	
	for (int i = 0; i < RD_DIAMETER; i++)
		for (int j = 0; j < RD_DIAMETER; j++) {
			printf("generating chunk at CX %i, CZ %i\n", i, j);
			loaded_chunks[i][j] = fetch_chunk(i + CHK_X, j + CHK_Z);
		}
	
	printf("chunks generated\n");
	
	mouse_event e;
	double rx, ry = 3.14159265359;
	
	mat4 m_mview = make_mview(c);
	mat4 m_proj = make_perspective(v.width / v.height, c.FOV, 0.1, 100);
	
	mat4 m_all = mmul4(m_mview, m_proj);
	
	for (;;) {
		e = get_mouse_event();
		ry -= e.rel_x / 128.0;
		rx += e.rel_y / 128.0;
		if (rx < -1.57079632679) rx = -1.57079632679;
		if (rx > 1.57079632679) rx = 1.57079632679;
		if (ry > 3.14159265359) ry -= 6.28318530718;
		if (ry <= -3.14159265359) ry += 6.28318530718;
		
		vec3 cz = norm3((vec3) {c.az.x, 0, c.az.z});
		vec3 cx = norm3((vec3) {c.ax.x, 0, c.ax.z});
		
		switch (get_char()) {
			case 'w': c.z += 1.5 * cz.z, c.x += 1.5 * cz.x; break;
			case 's': c.z -= 1.5 * cz.z, c.x -= 1.5 * cz.x; break;
			case 'a': c.x += 1.5 * cx.x, c.z += 1.5 * cx.z; break;
			case 'd': c.x -= 1.5 * cx.x, c.z -= 1.5 * cx.z; break;
			case ' ': case 'e': c.y += 1.5; break;
			case 'z': case 'q': c.y -= 1.5; break;
			case 'W': c.z += 2.5 * cz.z, c.x += 2.5 * cz.x; break;
			case 'S': c.z -= 2.5 * cz.z, c.x -= 2.5 * cz.x; break;
			case 'A': c.x += 2.5 * cx.x, c.z += 2.5 * cx.z; break;
			case 'D': c.x -= 2.5 * cx.x, c.z -= 2.5 * cx.z; break;
			case 'E': c.y -= 3; break;
			case 'Z': case 'Q': c.y -= 2.5; break;
			default: break;
		}
		
		rotate_camera(&c, rx, ry);
		
		update_chunks(c);
		
		raster_chunks(m_all, v, c);
		
		m_mview = make_mview(c);
		m_all = mmul4(m_mview, m_proj);
		
		raster_skybox(m_mview, v, c);
		
		for (int i = 0; i < fb_vinfo.yres; i++)
			memset(zbuf[i], 0x7F, fb_vinfo.xres * sizeof(float));
		
		fb_swap();
		//printf("\e[0;0Hrot: x %f, y %f    \npos: x %f, y %f, z %f    \nchk: x %i, z %i    \e[4;0H(%i, %i)\e[4;11H(%i, %i)\e[4;22H(%i, %i)    \e[5;0H(%i, %i)\e[5;11H(%i, %i)\e[5;22H(%i, %i)    \e[6;0H(%i, %i)\e[6;11H(%i, %i)\e[6;22H(%i, %i)    \n", rx / 3.1415926535 * 180.0, ry / 3.1415926535 * 180.0, c.x, c.y, c.z, CHK_X, CHK_Z/*, a[0][0].a, a[0][0].b, a[0][1].a, a[0][1].b, a[0][2].a, a[0][2].b, a[1][0].a, a[1][0].b, a[1][1].a, a[1][1].b, a[1][2].a, a[1][2].b, a[2][0].a, a[2][0].b, a[2][1].a, a[2][1].b, a[2][2].a, a[2][2].b*/);
	}
	
	return 0;
}