#ifndef __MCN_CHUNK_DEF_H__
#define __MCN_CHUNK_DEF_H__

#include <stdint.h>

#define CMOD16(X) ((X) < 0 ? 15 + ((int) (X) + 1) % 16 : (int) (X) % 16)
#define CREM16(X) ((X) < 0 ? ((int) (X) + 1) / 16 - 1 : (int) (X) / 16)

#define BYTE uint8_t
#define WORD uint16_t
#define DWORD uint32_t
#define QWORD uint64_t

#define RD 10
#define RD_DIAMETER 2 * RD + 1
#define BRD 16 * RD_DIAMETER - 1

#define BIOME_TEST	0
#define BIOME_RIVER 1

#define MAX_RIVERDEPTH 6

#define GET_BLOCK_FROM_LOADED_CHUNKS(CX, CZ, SEC, X, Y, Z) blocks[chks[CX][CZ]->sector[SEC].get[X][Y][Z].id]

#define CIXP ((x + 1) / 16)
#define RIXP ((x + 1) % 16)
#define SECP ((y + 1) / 16)
#define RIYP ((y + 1) % 16)
#define CIZP ((z + 1) / 16)
#define RIZP ((z + 1) % 16)
#define CIXN ((x - 1) / 16)
#define RIXN ((x - 1) % 16)
#define SECN ((y - 1) / 16)
#define RIYN ((y - 1) % 16)
#define CIZN ((z - 1) / 16)
#define RIZN ((z - 1) % 16)

struct {
	double FOV;
	double ASPECT_RATIO;
} client_config;

struct block_data {
	char* name;
	float mu_k;
	BYTE visibility : 2; // 00 = not visible, 01 = opague, 10 = translucent, 11 = binary transparency
	BYTE height_mod : 2; // for blocks that are slightly shorter
	BYTE is_solid	: 1; // can the block be collided with
	BYTE type		: 2; // 00 = regular, 01 = foliage, 10 = resvered, 11 = reserved
	WORD t_id[6]; // texture ids
};

struct block_data blocks[256] = {
	[0 ] = {"Air"		 	, 0.0, 0, 0, 0, 0, {0 , 0 , 0 , 0 , 0 , 0 }}, // air
	[1 ] = {"Grass"		 	, 0.5, 1, 0, 1, 0, {2 , 2 , 1 , 3 , 2 , 2 }}, // grass
	[2 ] = {"Stone"		 	, 0.5, 1, 0, 1, 0, {4 , 4 , 4 , 4 , 4 , 4 }}, // stone
	[3 ] = {"Dirt"		 	, 0.5, 1, 0, 1, 0, {3 , 3 , 3 , 3 , 3 , 3 }}, // dirt
	[4 ] = {"Oak Log"	 	, 0.5, 1, 0, 1, 0, {5 , 5 , 5 , 5 , 5 , 5 }}, // log
	[5 ] = {"Oak Leaves"	, 0.5, 1, 0, 1, 0, {6 , 6 , 6 , 6 , 6 , 6 }}, // leaves
	[6 ] = {"Still Water" 	, 0.5, 2, 2, 0, 0, {0 , 0 , 7 , 0 , 0 , 0 }}, // water
	[7 ] = {"Sand"		 	, 0.5, 1, 0, 1, 0, {8 , 8 , 8 , 8 , 8 , 8 }}, // sand
	[8 ] = {"Gravel"		, 0.5, 1, 0, 1, 0, {9 , 9 , 9 , 9 , 9 , 9 }}, // gravel
	[9 ] = {"Wild Grass"	, 0.0, 3, 0, 0, 1, {10, 10, 10, 10, 10, 10}}, // wild grass
};

typedef struct {
	DWORD seed; // world seed
	// ...
} world;

typedef struct __attribute__((__packed__)) {
	BYTE id : 4; // block id
	BYTE light: 4; // light level of block
	BYTE skylight : 1; // set if block has access to skylight
	BYTE reserved : 3; // reserved
	// ...
} voxel;

typedef struct {
	voxel get[16][16][16];
	// ...
} sector;

typedef struct {
	BYTE id;
	float purity;
} biome_descriptor;

typedef struct {
	biome_descriptor biome_map[16][16];
	sector sector[8]; // list of sectors (which contain voxel data)
	BYTE sector_info; // ech bit is unset if corresponding sector is empty
	int x, z; // x and z position of chunk (in chunks)
	// ...
} chunk;

typedef struct {
	BYTE x, y, z; // x y z coordinate of block (in relative chunk coordinates)
	BYTE id;
	BYTE face_xp : 1; // set if face is visible
	BYTE face_xn : 1;
	BYTE face_yp : 1;
	BYTE face_yn : 1;
	BYTE face_zp : 1;
	BYTE face_zn : 1;
	BYTE face_am : 1;
	BYTE face_as : 1;
	WORD occ000  : 2; // vertex occlusion values
	WORD occ001  : 2; // (yes, values are stored)
	WORD occ010  : 2; // (even if no face uses  )
	WORD occ011  : 2; // (the vertex			)
	WORD occ100  : 2;
	WORD occ101  : 2;
	WORD occ110  : 2;
	WORD occ111  : 2;
} active_block;

typedef struct {
	active_block* get;
	int len;
} chunk_mesh;

typedef struct p_c_m {
	active_block block;
	struct p_c_m* next;
	struct p_c_m* prev;
} pre_chunk_mesh;

typedef struct {
	chunk data;
	chunk_mesh mesh;
	chunk_mesh deferred_mesh;
} loaded_chunk;

chunk* chks[RD_DIAMETER][RD_DIAMETER];
chunk_mesh chk_meshes[RD_DIAMETER][RD_DIAMETER];
chunk_mesh chk_deferred_meshes[RD_DIAMETER][RD_DIAMETER];

int CHK_X, CHK_Z;
int OLD_CHK_X, OLD_CHK_Z;

chunk* generate_chunk(int _chunk_x, int _chunk_z) {
	int depth, block;
	int num_trees, tree_height;

	chunk* chk = calloc(1, sizeof(chunk));

	chk->x = _chunk_x;
	chk->z = _chunk_z;

	char heightmap[16][16];
	biome_descriptor biome_map[16][16];

	double pval = 0;

	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			pval = perlin(x + chk->x * 16, z + chk->z * 16, 0.5);

			if (pval > -0.1) {
				biome_map[x][z] = (biome_descriptor) {BIOME_TEST, FABS(pval * 0.909091)};
				heightmap[x][z] = (int) (16 * pval) + 32;
				if (pval < -0.025) {
					if (perlin(x + chk->x * 16, z + chk->z * 16, heightmap[x][z] + 0.5) > 0.2) block = 8;
					else block = 7;

					chk->sector[(heightmap[x][z] + 1) / 16].get[x][(heightmap[x][z] + 1) % 16][z] = (voxel) {block};

					depth = pr_range(2, 5);
					for (int y = heightmap[x][z] - depth; y <= heightmap[x][z]; y++)
						chk->sector[y / 16].get[x][y % 16][z] = (voxel) {block};

					heightmap[x][z] -= depth;
				} else {
					chk->sector[(heightmap[x][z] + 1) / 16].get[x][(heightmap[x][z] + 1) % 16][z] = (voxel) {1};
					if (perlin(8 * (x + chk->x * 16), 8 * (z + chk->z * 16), 420.69) > 0)
						chk->sector[(heightmap[x][z] + 2) / 16].get[x][(heightmap[x][z] + 2) % 16][z] = (voxel) {9};
				}
			} else {
				biome_map[x][z] = (biome_descriptor) {BIOME_RIVER, FABS(pval * 1.111111)};
				heightmap[x][z] = 31;

				chk->sector[(heightmap[x][z] + 1) / 16].get[x][(heightmap[x][z] + 1) % 16][z] = (voxel) {6};

				if (perlin(x + chk->x * 16, z + chk->z * 16, heightmap[x][z] + 0.5) > 0.2) block = 8;
				else block = 7;

				heightmap[x][z] -= (double) MAX_RIVERDEPTH * biome_map[x][z].purity;
				chk->sector[(heightmap[x][z] + 1) / 16].get[x][(heightmap[x][z] + 1) % 16][z] = (voxel) {block};

				depth = pr_range(2, 5);
				for (int y = heightmap[x][z] - depth; y <= heightmap[x][z]; y++)
					chk->sector[y / 16].get[x][y % 16][z] = (voxel) {block};

				heightmap[x][z] -= depth;
			}

			depth = pr_range(2, 5);
			for (int y = heightmap[x][z]; y >= 0; y--) {
				if (depth > 0) {
					chk->sector[y / 16].get[x][y % 16][z] = (voxel) {3};
					depth--;
				} else {
					chk->sector[y / 16].get[x][y % 16][z] = (voxel) {2};
				}
			}

			for (int y = 127; blocks[chk->sector[y / 16].get[x][y % 16][z].id].visibility != 1 && y >= 0; y--)
				chk->sector[y / 16].get[x][y % 16][z].skylight = 1;
		}
	}

	num_trees = pr_range(1, 2);

	for (int k = 0; k < num_trees; k++) {
		int x = pr_range(2, 13), z = pr_range(2, 13);
		if (biome_map[x][z].id != BIOME_TEST) continue;
		if (chk->sector[(heightmap[x][z] + 1) / 16].get[x][(heightmap[x][z] + 1) % 16][z].id != 1) continue;

		tree_height = pr_range(5, 6);
		for (int i = tree_height; i >= 0; i--)
			chk->sector[(heightmap[x][z] + i) / 16].get[x][(heightmap[x][z] + i) % 16][z] = (voxel) {4};

		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x + 0][(heightmap[x][z] + tree_height - 2) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x + 0][(heightmap[x][z] + tree_height - 2) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x + 0][(heightmap[x][z] + tree_height - 2) % 16][z - 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x + 0][(heightmap[x][z] + tree_height - 2) % 16][z + 2] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x + 0][(heightmap[x][z] + tree_height - 2) % 16][z - 2] = (voxel) {5};

		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x + 1][(heightmap[x][z] + tree_height - 2) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x + 1][(heightmap[x][z] + tree_height - 2) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x + 1][(heightmap[x][z] + tree_height - 2) % 16][z - 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x + 1][(heightmap[x][z] + tree_height - 2) % 16][z + 2] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x + 1][(heightmap[x][z] + tree_height - 2) % 16][z - 2] = (voxel) {5};

		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x - 1][(heightmap[x][z] + tree_height - 2) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x - 1][(heightmap[x][z] + tree_height - 2) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x - 1][(heightmap[x][z] + tree_height - 2) % 16][z - 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x - 1][(heightmap[x][z] + tree_height - 2) % 16][z + 2] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x - 1][(heightmap[x][z] + tree_height - 2) % 16][z - 2] = (voxel) {5};

		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x + 2][(heightmap[x][z] + tree_height - 2) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x + 2][(heightmap[x][z] + tree_height - 2) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x + 2][(heightmap[x][z] + tree_height - 2) % 16][z - 1] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 2) % 16][z + 2] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 2) % 16][z - 2] = (voxel) {5};

		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x - 2][(heightmap[x][z] + tree_height - 2) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x - 2][(heightmap[x][z] + tree_height - 2) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 2) / 16].get[x - 2][(heightmap[x][z] + tree_height - 2) % 16][z - 1] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 2) % 16][z + 2] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 2) % 16][z - 2] = (voxel) {5};


		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x + 0][(heightmap[x][z] + tree_height - 1) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x + 0][(heightmap[x][z] + tree_height - 1) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x + 0][(heightmap[x][z] + tree_height - 1) % 16][z - 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x + 0][(heightmap[x][z] + tree_height - 1) % 16][z + 2] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x + 0][(heightmap[x][z] + tree_height - 1) % 16][z - 2] = (voxel) {5};

		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x + 1][(heightmap[x][z] + tree_height - 1) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x + 1][(heightmap[x][z] + tree_height - 1) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x + 1][(heightmap[x][z] + tree_height - 1) % 16][z - 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x + 1][(heightmap[x][z] + tree_height - 1) % 16][z + 2] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x + 1][(heightmap[x][z] + tree_height - 1) % 16][z - 2] = (voxel) {5};

		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x - 1][(heightmap[x][z] + tree_height - 1) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x - 1][(heightmap[x][z] + tree_height - 1) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x - 1][(heightmap[x][z] + tree_height - 1) % 16][z - 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x - 1][(heightmap[x][z] + tree_height - 1) % 16][z + 2] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x - 1][(heightmap[x][z] + tree_height - 1) % 16][z - 2] = (voxel) {5};

		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x + 2][(heightmap[x][z] + tree_height - 1) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x + 2][(heightmap[x][z] + tree_height - 1) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x + 2][(heightmap[x][z] + tree_height - 1) % 16][z - 1] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 1) % 16][z + 2] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x + 2][(heightmap[x][z] + tree_height - 1) % 16][z - 2] = (voxel) {5};

		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x - 2][(heightmap[x][z] + tree_height - 1) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x - 2][(heightmap[x][z] + tree_height - 1) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 1) / 16].get[x - 2][(heightmap[x][z] + tree_height - 1) % 16][z - 1] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 1) % 16][z + 2] = (voxel) {5};
		if (!(rand() % 6)) chk->sector[heightmap[x][z] / 16].get[x - 2][(heightmap[x][z] + tree_height - 1) % 16][z - 2] = (voxel) {5};


		chk->sector[(heightmap[x][z] + tree_height - 0) / 16].get[x + 0][(heightmap[x][z] + tree_height - 0) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 0) / 16].get[x + 0][(heightmap[x][z] + tree_height - 0) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 0) / 16].get[x + 0][(heightmap[x][z] + tree_height - 0) % 16][z - 1] = (voxel) {5};

		chk->sector[(heightmap[x][z] + tree_height - 0) / 16].get[x + 1][(heightmap[x][z] + tree_height - 0) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 0) / 16].get[x + 1][(heightmap[x][z] + tree_height - 0) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 0) / 16].get[x + 1][(heightmap[x][z] + tree_height - 0) % 16][z - 1] = (voxel) {5};

		chk->sector[(heightmap[x][z] + tree_height - 0) / 16].get[x - 1][(heightmap[x][z] + tree_height - 0) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 0) / 16].get[x - 1][(heightmap[x][z] + tree_height - 0) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height - 0) / 16].get[x - 1][(heightmap[x][z] + tree_height - 0) % 16][z - 1] = (voxel) {5};


		chk->sector[(heightmap[x][z] + tree_height + 1) / 16].get[x + 0][(heightmap[x][z] + tree_height + 1) % 16][z + 0] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height + 1) / 16].get[x + 0][(heightmap[x][z] + tree_height + 1) % 16][z + 1] = (voxel) {5};
		chk->sector[(heightmap[x][z] + tree_height + 1) / 16].get[x + 0][(heightmap[x][z] + tree_height + 1) % 16][z - 1] = (voxel) {5};

		chk->sector[(heightmap[x][z] + tree_height + 1) / 16].get[x + 1][(heightmap[x][z] + tree_height + 1) % 16][z + 0] = (voxel) {5};

		chk->sector[(heightmap[x][z] + tree_height + 1) / 16].get[x - 1][(heightmap[x][z] + tree_height + 1) % 16][z + 0] = (voxel) {5};
	}

	return chk;
}

void init_chunks() {
	static active_block block;
	
	static int n_xp; // neighbouring voxels
	static int n_xn; // block id
	static int n_yp;
	static int n_yn;
	static int n_zp;
	static int n_zn;
	
	static int c_id;
	static int is_visible;
	
	static int block_counts[RD_DIAMETER][RD_DIAMETER] = {0};
	static int deferred_block_counts[RD_DIAMETER][RD_DIAMETER] = {0};
	
	static pre_chunk_mesh* proto_meshes[RD_DIAMETER][RD_DIAMETER] = {0};
	static pre_chunk_mesh* deferred_proto_meshes[RD_DIAMETER][RD_DIAMETER] = {0};
	
	for (int cx = 0; cx < RD_DIAMETER; cx++)
	for (int cz = 0; cz < RD_DIAMETER; cz++) {
		proto_meshes[cx][cz] = calloc(1, sizeof(pre_chunk_mesh));
		deferred_proto_meshes[cx][cz] = calloc(1, sizeof(pre_chunk_mesh));
		block_counts[cx][cz] = 0;
		deferred_block_counts[cx][cz] = 0;
	}
	
	for (int x = 0; x < 16 * RD_DIAMETER; x++)
	for (int z = 0; z < 16 * RD_DIAMETER; z++)
	for (int y = 0; y < 128; y++) {
		int cx = x / 16, cz = z / 16, sec = y / 16;
		int rx = x % 16, rz = z % 16,  ry = y % 16;
		c_id = chks[cx][cz]->sector[sec].get[rx][ry][rz].id;
		
		if (blocks[c_id].visibility) {
			block = (active_block) {rx, y, rz, c_id};
			is_visible = 0;
			
			n_xp = (x < BRD) ? chks[CIXP][cz  ]->sector[sec ].get[RIXP][ry  ][rz  ].id : 1;
			n_xn = (x > 0  ) ? chks[CIXN][cz  ]->sector[sec ].get[RIXN][ry  ][rz  ].id : 1;
			n_yp = (y < 127) ? chks[cx  ][cz  ]->sector[SECP].get[rx  ][RIYP][rz  ].id : 1;
			n_yn = (y > 0  ) ? chks[cx  ][cz  ]->sector[SECN].get[rx  ][RIYN][rz  ].id : 1;
			n_zp = (z < BRD) ? chks[cx  ][CIZP]->sector[sec ].get[rx  ][ry  ][RIZP].id : 1;
			n_zn = (z > 0  ) ? chks[cx  ][CIZN]->sector[sec ].get[rx  ][ry  ][RIZN].id : 1;
			
			if (blocks[n_xp].visibility != 1) block.face_xp = 1, is_visible = 1;
			if (blocks[n_xn].visibility != 1) block.face_xn = 1, is_visible = 1;
			if (blocks[n_yp].visibility != 1) block.face_yp = 1, is_visible = 1;
			if (blocks[n_yn].visibility != 1) block.face_yn = 1, is_visible = 1;
			if (blocks[n_zp].visibility != 1) block.face_zp = 1, is_visible = 1;
			if (blocks[n_zn].visibility != 1) block.face_zn = 1, is_visible = 1;
			
			if (is_visible) {
				if (x < BRD && y < 127            && GET_BLOCK_FROM_LOADED_CHUNKS(CIXP, cz	, SECP, RIXP, RIYP, rz	).visibility == 1) block.occ110++, block.occ111++   ; // +x, +y,  z
				if (x > 0   && y < 127            && GET_BLOCK_FROM_LOADED_CHUNKS(cx  , cz	, SECP, rx  , RIYP, rz	).visibility == 1) block.occ010++, block.occ011++   ; // -x, +y,  z
				if (           y < 127 && z < BRD && GET_BLOCK_FROM_LOADED_CHUNKS(cx  , CIZP, SECP, rx	, RIYP, RIZP).visibility == 1) block.occ011++, block.occ111++   ; //  x, +y, +z
				if (           y < 127 && z > 0   && GET_BLOCK_FROM_LOADED_CHUNKS(cx  , cz  , SECP, rx	, RIYP, rz  ).visibility == 1) block.occ010++, block.occ110++   ; //  x, +y, -z
				if (x < BRD                       && GET_BLOCK_FROM_LOADED_CHUNKS(CIXP, cz	, sec , RIXP, ry  , rz	).visibility == 1) block.occ100++, block.occ101++   ; // +x,  y,  z
				if (x > 0                         && GET_BLOCK_FROM_LOADED_CHUNKS(cx  , cz	, sec , rx  , ry  , rz	).visibility == 1) block.occ000++, block.occ001++   ; // -x,  y,  z
				if (                      z < BRD && GET_BLOCK_FROM_LOADED_CHUNKS(cx  , CIZP, sec , rx	, ry  , RIZP).visibility == 1) block.occ001++, block.occ101++   ; //  x,  y, +z
				if (                      z > 0   && GET_BLOCK_FROM_LOADED_CHUNKS(cx  , cz  , sec , rx	, ry  , rz  ).visibility == 1) block.occ000++, block.occ100++   ; //  x,  y, -z
				
				if (block.occ111 < 2 && x < BRD && y < 127 && z < BRD && GET_BLOCK_FROM_LOADED_CHUNKS(CIXP, CIZP, SECP, RIXP, RIYP, RIZP).visibility == 1) block.occ111++ ; // +x, +y, +z
				if (block.occ011 < 2 && x > 0   && y < 127 && z < BRD && GET_BLOCK_FROM_LOADED_CHUNKS(cx  , CIZP, SECP, rx  , RIYP, RIZP).visibility == 1) block.occ011++ ; // -x, +y, +z
				if (block.occ110 < 2 && x < BRD && y < 127 && z > 0   && GET_BLOCK_FROM_LOADED_CHUNKS(CIXP, cz  , SECP, RIXP, RIYP, rz  ).visibility == 1) block.occ110++ ; // +x, +y, -z
				if (block.occ010 < 2 && x > 0   && y < 127 && z > 0   && GET_BLOCK_FROM_LOADED_CHUNKS(cx  , cz  , SECP, rx  , RIYP, rz  ).visibility == 1) block.occ010++ ; // -x, +y, -z
				if (block.occ101 < 2 && x < BRD &&            z < BRD && GET_BLOCK_FROM_LOADED_CHUNKS(CIXP, CIZP, sec , RIXP, ry  , RIZP).visibility == 1) block.occ101++ ; // +x,  y, +z
				if (block.occ001 < 2 && x > 0   &&            z < BRD && GET_BLOCK_FROM_LOADED_CHUNKS(cx  , CIZP, sec , rx  , ry  , RIZP).visibility == 1) block.occ001++ ; // -x,  y, +z
				if (block.occ100 < 2 && x < BRD &&            z > 0   && GET_BLOCK_FROM_LOADED_CHUNKS(CIXP, cz  , sec , RIXP, ry  , rz  ).visibility == 1) block.occ100++ ; // +x,  y, -z
				if (block.occ000 < 2 && x > 0   &&            z > 0   && GET_BLOCK_FROM_LOADED_CHUNKS(cx  , cz  , sec , rx  , ry  , rz  ).visibility == 1) block.occ000++ ; // -x,  y, -z
				
				switch (blocks[c_id].visibility) {
					case 3:
						block.face_am = 1;
						block.face_as = 1;
					case 1:
						proto_meshes[cx][cz]->block = block;
						proto_meshes[cx][cz]->next = calloc(1, sizeof(pre_chunk_mesh));
						proto_meshes[cx][cz]->next->prev = proto_meshes[cx][cz];
						proto_meshes[cx][cz] = proto_meshes[cx][cz]->next;
						block_counts[cx][cz]++;
						break;
					case 2:
						deferred_proto_meshes[cx][cz]->block = block;
						deferred_proto_meshes[cx][cz]->next = calloc(1, sizeof(pre_chunk_mesh));
						deferred_proto_meshes[cx][cz]->next->prev = deferred_proto_meshes[cx][cz];
						deferred_proto_meshes[cx][cz] = deferred_proto_meshes[cx][cz]->next;
						deferred_block_counts[cx][cz]++;
						break;
					default:
						break;
				}
			}
		}
	}
	
	for (int cx = 0; cx < RD_DIAMETER; cx++)
	for (int cz = 0; cz < RD_DIAMETER; cz++) {
		// free prev meshes
		free(chk_meshes[cx][cz].get);
		free(chk_deferred_meshes[cx][cz].get);
		
		// generate array from chunk mesh data
		chk_meshes[cx][cz].get = malloc(block_counts[cx][cz] * sizeof(active_block));
		chk_meshes[cx][cz].len = block_counts[cx][cz];
		
		for (int i = 0; proto_meshes[cx][cz]->prev; i++) {
			proto_meshes[cx][cz] = proto_meshes[cx][cz]->prev;
			free(proto_meshes[cx][cz]->next);
			chk_meshes[cx][cz].get[i] = proto_meshes[cx][cz]->block;
		}
		
		free(proto_meshes[cx][cz]);
		
		// generate array from deferred chunk mesh data
		chk_deferred_meshes[cx][cz].get = malloc(deferred_block_counts[cx][cz] * sizeof(active_block));
		chk_deferred_meshes[cx][cz].len = deferred_block_counts[cx][cz];
		
		for (int i = 0; deferred_proto_meshes[cx][cz]->prev; i++) {
			deferred_proto_meshes[cx][cz] = deferred_proto_meshes[cx][cz]->prev;
			free(deferred_proto_meshes[cx][cz]->next);
			chk_deferred_meshes[cx][cz].get[i] = deferred_proto_meshes[cx][cz]->block;
		}
		
		free(deferred_proto_meshes[cx][cz]);
	}
}

chunk* fetch_chunk(int _chunk_x, int _chunk_z) {
	char chunk_name[256];
	sprintf(chunk_name, "chunk_data/%i_%i_mne_chk", _chunk_x, _chunk_z);
	chunk* chk;

	chk = generate_chunk(_chunk_x, _chunk_z);

	/*if (access(chunk_name, F_OK)) {
		chk = generate_chunk(_chunk_x, _chunk_z);
		int fd = creat(chunk_name, O_WRONLY);
		write(fd, chk, sizeof(chunk));
		close(fd);
	} else {
		//printf("! chunk CX %i, CZ %i already exists: loading...\n", _chunk_x, _chunk_z);
		int fd = open(chunk_name, O_RDONLY);
		chk = malloc(sizeof(chunk));
		read(fd, chk, sizeof(chunk));
		close(fd);
	}*/

	return chk;
}

int update_chunks(double _ctr_x, double _ctr_z) {
		CHK_X = CREM16(_ctr_x);
		CHK_Z = CREM16(_ctr_z);

		if (CHK_X > OLD_CHK_X) {
			for (int i = 0; i < RD_DIAMETER - 1; i++)
				for (int j = 0; j < RD_DIAMETER; j++)
					chks[i][j] = chks[i + 1][j];
			for (int i = 0; i < RD_DIAMETER; i++)
				chks[RD_DIAMETER - 1][i] = fetch_chunk(CHK_X + RD_DIAMETER - 1, OLD_CHK_Z + i);
			OLD_CHK_X = CHK_X;
		
			init_chunks();
		} else if (CHK_X < OLD_CHK_X) {
			for (int i = RD_DIAMETER - 1; i > 0; i--)
				for (int j = 0; j < RD_DIAMETER; j++)
					chks[i][j] = chks[i - 1][j];
			for (int i = 0; i < RD_DIAMETER; i++)
				chks[0][i] = fetch_chunk(CHK_X, OLD_CHK_Z + i);
			OLD_CHK_X = CHK_X;
		
			init_chunks();
		}

		if (CHK_Z > OLD_CHK_Z) {
			for (int i = 0; i < RD_DIAMETER; i++)
				for (int j = 0; j < RD_DIAMETER - 1; j++)
					chks[i][j] = chks[i][j + 1];
			for (int i = 0; i < RD_DIAMETER; i++)
				chks[i][RD_DIAMETER - 1] = fetch_chunk(OLD_CHK_X + i, CHK_Z + RD_DIAMETER - 1);
			OLD_CHK_Z = CHK_Z;
		
			init_chunks();
		} else if (CHK_Z < OLD_CHK_Z) {
			for (int i = 0; i < RD_DIAMETER; i++)
				for (int j = RD_DIAMETER - 1; j > 0; j--)
					chks[i][j] = chks[i][j - 1];
			for (int i = 0; i < RD_DIAMETER; i++)
				chks[i][0] = fetch_chunk(OLD_CHK_X + i, CHK_Z);
			OLD_CHK_Z = CHK_Z;
		
			init_chunks();
		}
}

#endif