#include "inc/graphics.h"
#include "inc/mouse.h"

#include <stdio.h>
#include <termios.h>

#include "inc/perlin.h"

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

typedef struct {
	tri tri[12];
} cube;

cube make_cube(vertex* v000, vertex* v001, vertex* v010, vertex* v011, vertex* v100, vertex* v101, vertex* v110, vertex* v111) {
	return (cube) {
		(tri) {v000, v010, v100},
		(tri) {v110, v100, v010},
		(tri) {v001, v101, v011},
		(tri) {v111, v011, v101},
		(tri) {v000, v001, v010},
		(tri) {v011, v010, v001},
		(tri) {v100, v110, v101},
		(tri) {v111, v101, v110},
		(tri) {v000, v100, v001},
		(tri) {v101, v001, v100},
		(tri) {v010, v011, v110},
		(tri) {v111, v110, v011},
	};
}

void raster_cube(cube _c, viewport* _v, mat4 _mv, mat4 _mp) {
	raster_tri(_c.tri[0], _v, _mv, _mp);
	raster_tri(_c.tri[1], _v, _mv, _mp);
	raster_tri(_c.tri[2], _v, _mv, _mp);
	raster_tri(_c.tri[3], _v, _mv, _mp);
	raster_tri(_c.tri[4], _v, _mv, _mp);
	raster_tri(_c.tri[5], _v, _mv, _mp);
	raster_tri(_c.tri[6], _v, _mv, _mp);
	raster_tri(_c.tri[7], _v, _mv, _mp);
	raster_tri(_c.tri[8], _v, _mv, _mp);
	raster_tri(_c.tri[9], _v, _mv, _mp);
	raster_tri(_c.tri[10], _v, _mv, _mp);
	raster_tri(_c.tri[11], _v, _mv, _mp);
}

chunk* generate_chunk(int _chunk_x, int _chunk_z) {
	int num_trees, tree_height, dirt_depth, gen_height;
	
	chunk* chk = calloc(1, sizeof(chunk));
	
	chk->x = _chunk_x;
	chk->z = _chunk_z;
	
	char heightmap[16][16];
	
	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			gen_height = (int) (48 * fabs(perlin(((double) (x + chk->x * 16)) / 16, ((double) (z + chk->z * 16)) / 16, 0.5))) + 32;
			heightmap[x][z] = gen_height + 1;
			chk->sector[gen_height / 16].get[x][gen_height % 16 + 1][z] = (voxel) {1};
			dirt_depth = rand() % 4 + 2; // rand between 2 and 5
			for (int y = gen_height; y >= 0; y--) {
				if (dirt_depth > 0) {
					chk->sector[y / 16].get[x][y % 16][z] = (voxel) {3};
					dirt_depth--;
				} else {
					chk->sector[y / 16].get[x][y % 16][z] = (voxel) {2};
				}
			}
		}
	}
	
	num_trees = rand() % 2 + 1;
	
	for (int k = 0; k < num_trees; k++) {
		int x = rand() % 12 + 2, z = rand() % 12 + 2;
		
		tree_height = rand() % 3 + 4; // rand between 4 and 6
		for (int i = tree_height; i >= 0; i--)
			chk->sector[heightmap[x][z] / 16].get[x][(heightmap[x][z] + i) % 16][z] = (voxel) {4};
		
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height - 2) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height - 2) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height - 2) % 16][z - 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height - 2) % 16][z + 2] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height - 2) % 16][z - 2] = (voxel) {5};
		
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height - 2) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height - 2) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height - 2) % 16][z - 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height - 2) % 16][z + 2] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height - 2) % 16][z - 2] = (voxel) {5};
		
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height - 2) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height - 2) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height - 2) % 16][z - 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height - 2) % 16][z + 2] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height - 2) % 16][z - 2] = (voxel) {5};
		
		chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 2) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 2) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 2) % 16][z - 1] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 2) % 16][z + 2] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 2) % 16][z - 2] = (voxel) {5};
		
		chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 2) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 2) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 2) % 16][z - 1] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 2) % 16][z + 2] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 2) % 16][z - 2] = (voxel) {5};
		
		
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height - 1) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height - 1) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height - 1) % 16][z - 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height - 1) % 16][z + 2] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height - 1) % 16][z - 2] = (voxel) {5};
		
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height - 1) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height - 1) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height - 1) % 16][z - 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height - 1) % 16][z + 2] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height - 1) % 16][z - 2] = (voxel) {5};
		
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height - 1) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height - 1) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height - 1) % 16][z - 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height - 1) % 16][z + 2] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height - 1) % 16][z - 2] = (voxel) {5};
		
		chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 1) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 1) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 1) % 16][z - 1] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 1) % 16][z + 2] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 1) % 16][z - 2] = (voxel) {5};
		
		chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 1) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 1) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 1) % 16][z - 1] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 1) % 16][z + 2] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 1) % 16][z - 2] = (voxel) {5};
		
		
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height - 0) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height - 0) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height - 0) % 16][z - 1] = (voxel) {5};
		
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height - 0) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height - 0) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height - 0) % 16][z - 1] = (voxel) {5};
		
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height - 0) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height - 0) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height - 0) % 16][z - 1] = (voxel) {5};
		
		
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height + 1) % 16][z + 0] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height + 1) % 16][z + 1] = (voxel) {5};
		chk->sector[heightmap[x][z] / 16].get[x + 0][(heightmap[x][z] + tree_height + 1) % 16][z - 1] = (voxel) {5};
		
		chk->sector[heightmap[x][z] / 16].get[x + 1][(heightmap[x][z] + tree_height + 1) % 16][z + 0] = (voxel) {5};
		
		chk->sector[heightmap[x][z] / 16].get[x - 1][(heightmap[x][z] + tree_height + 1) % 16][z + 0] = (voxel) {5};
	}
	
	return chk;
}

chunk* fetch_chunk(int _chunk_x, int _chunk_z) {
	char chunk_name[256];
	sprintf(chunk_name, "chunk_data/%i_%i_mne_chk", _chunk_x, _chunk_z);
	chunk* chk;
	
	if (access(chunk_name, F_OK)) {
		chk = generate_chunk(_chunk_x, _chunk_z);
		int fd = creat(chunk_name, O_WRONLY);
		write(fd, chk, sizeof(chunk));
		close(fd);
	} else {
		printf("! chunk CX %i, CZ %i already exists: loading...\n", _chunk_x, _chunk_z);
		int fd = open(chunk_name, O_RDONLY);
		chk = malloc(sizeof(chunk));
		read(fd, chk, sizeof(chunk));
		close(fd);
	}
	
	return chk;
}

int main(void) {

    struct termios term;
    tcgetattr(fileno(stdin), &term);

    term.c_lflag |= ECHO;
    tcsetattr(fileno(stdin), 0, &term);
	
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
	
	if (mouse_init("/dev/input/mice")) printf("error: failed to open mouse device");
	
	g_tx0 = import_bitmap("tx0.bmp");
	g_tx1 = import_bitmap("tx1.bmp");
	g_tx2 = import_bitmap("tx2.bmp");
	
	g_sb_xp = import_bitmap("xp.bmp");
	g_sb_xn = import_bitmap("xn.bmp");
	g_sb_yp = import_bitmap("yp.bmp");
	g_sb_yn = import_bitmap("yn.bmp");
	g_sb_zp = import_bitmap("zp.bmp");
	g_sb_zn = import_bitmap("zn.bmp");
	
	textures[0] = import_bitmap("grass_top.bmp");
	textures[1] = import_bitmap("grass_side.bmp");
	textures[2] = import_bitmap("grass_bottom.bmp");
	textures[3] = import_bitmap("stone.bmp");
	textures[4] = import_bitmap("log.bmp");
	textures[5] = import_bitmap("leaves.bmp");
	textures[6] = import_bitmap("water_still.bmp");
	
	int r = 10;
	
	zbuf = malloc(fb_vinfo.yres * sizeof(void*));
	for (int i = 0; i < fb_vinfo.yres; i++) {
		zbuf[i] = malloc(fb_vinfo.xres * sizeof(int));
		for (int j = 0; j < fb_vinfo.xres; j++)
			zbuf[i][j] = 1000000;
	}
	
	vertex v0 = (vertex) {0, 0, 0, 0, 0, 0};
	vertex v1 = (vertex) {0, 0, r, 0, 0, 0};
	vertex v2 = (vertex) {0, r, 0, 0, 0, 0};
	vertex v3 = (vertex) {0, r, r, 0, 0, 0};
	vertex v4 = (vertex) {r, 0, 0, 0, 0, 0};
	vertex v5 = (vertex) {r, 0, r, 0, 0, 0};
	vertex v6 = (vertex) {r, r, 0, 0, 0, 0};
	vertex v7 = (vertex) {r, r, r, 0, 0, 0};
	
	cube cu = make_cube(&v0, &v1, &v2, &v3, &v4, &v5, &v6, &v7);
	
	viewport v = {800, 600, 0, 0};
	camera c = {-8.1, 32.1, -8.1};
	
	CUR_CHK_X = c.x / 16 + RD - (c.x < 0);
	CUR_CHK_Z = c.z / 16 + RD - (c.z < 0);
	
	int OLD_CUR_CHK_X = CUR_CHK_X;
	int OLD_CUR_CHK_Z = CUR_CHK_Z;
	
	mat4 w = make_mview(&c);
	mat4 p = make_perspective(8/6, 90, 0.1, 100);
	mat4 mp_inv = make_perspective_inv(8/6, 90, 0.1, 100);
	
	mat4 m = mmul4(w, p);
	
	mouse_event e;
	double rx, ry = 3.14159265359;
	
	printf("generating chunks\n");
	
	
	
	srand(time(NULL));
	pl_init(125725, 3, 1.5, 0.6);
	
	
	for (int i = 0; i < RD_DIAMETER; i++) {
		for (int j = 0; j < RD_DIAMETER; j++) {
			printf("generating chunk at CX %i, CZ %i\n", i, j);
			chks[i][j] = fetch_chunk(i, j);
		}
	}
	
	printf("chunks generated\n");
	
	for(;;) {
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
		
		CUR_CHK_X = c.x / 16 + RD - (c.x < 0);
		CUR_CHK_Z = c.z / 16 + RD - (c.z < 0);
		
		if (CUR_CHK_X > OLD_CUR_CHK_X) {
			for (int i = RD_DIAMETER - 1; i > 0; i--)
				for (int j = 0; j < RD_DIAMETER; j++)
					chks[i][j] = chks[i - 1][j];
			for (int i = 0; i < RD_DIAMETER; i++) {
				chks[0][i] = fetch_chunk(CUR_CHK_X + RD, CUR_CHK_Z - RD + i);
			}
			OLD_CUR_CHK_X = CUR_CHK_X;
		}
		
		if (CUR_CHK_X < OLD_CUR_CHK_X) {
			for (int i = 0; i < RD_DIAMETER - 1; i++)
				for (int j = 0; j < RD_DIAMETER; j++)
					chks[i][j] = chks[i + 1][j];
			for (int i = 0; i < RD_DIAMETER; i++) {
				chks[RD_DIAMETER - 1][i] = fetch_chunk(CUR_CHK_X - RD, CUR_CHK_Z - RD + i);
			}
			OLD_CUR_CHK_X = CUR_CHK_X;
		}
		
		if (CUR_CHK_Z > OLD_CUR_CHK_Z) {
			for (int i = RD_DIAMETER - 1; i > 0; i--)
				for (int j = 0; j < RD_DIAMETER; j++)
					chks[j][i] = chks[j][i - 1];
			for (int i = 0; i < RD_DIAMETER; i++) {
				chks[i][0] = fetch_chunk(CUR_CHK_X - RD + i, CUR_CHK_Z + RD);
			}
			OLD_CUR_CHK_Z = CUR_CHK_Z;
		}
		
		if (CUR_CHK_Z < OLD_CUR_CHK_Z) {
			for (int i = 0; i < RD_DIAMETER - 1; i++)
				for (int j = 0; j < RD_DIAMETER; j++)
					chks[j][i] = chks[j][i + 1];
			for (int i = 0; i < RD_DIAMETER; i++) {
				chks[i][RD_DIAMETER - 1] = fetch_chunk(CUR_CHK_X - RD + i, CUR_CHK_Z - RD);
			}
			OLD_CUR_CHK_Z = CUR_CHK_Z;
		}
		
		rotate_camera(&c, rx, ry);
		//w = make_mview(&c);
		
		mat4 mv_inv = make_mview_inv(&c);
		
		draw(mv_inv, &c, &v, chks[0][0]);
		
		fb_swap();
		//raster_cube(cu, &v, w, p);
		//raster_tri((tri) {&v0, &v2, &v4}, &v, w, p);
		//fb_swap();
		// for (int i = 0; i < fb_vinfo.yres; i++) {
			// for (int j = 0; j < fb_vinfo.xres; j++)
				// zbuf[i][j] = 1000000;
		// }
		printf("\e[0;0Hrot: x %f, y %f\npos: x %f, y %f, z %f\n", rx / 3.1415926535 * 180.0, ry / 3.1415926535 * 180.0, c.x, c.y, c.z);;
	}
}