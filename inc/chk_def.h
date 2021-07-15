#ifndef __MCN_CHUNK_DEF_H__
#define __MCN_CHUNK_DEF_H__

#include <stdint.h>

#define CMOD16(X) (X < 0 ? 15 + ((int) X + 1) % 16 : (int) X % 16)
#define CREM16(X) (X < 0 ? ((int) X + 1) / 16 - 1 : (int) X / 16)

#define BYTE uint8_t
#define WORD uint16_t
#define DWORD uint32_t
#define QWORD uint64_t

#define RD 3
#define RD_DIAMETER 2 * RD + 1
#define VRD 16 * RD_DIAMETER - 1

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
	BYTE id : 4;
	BYTE light : 4;
	// ...
} voxel;

typedef struct {
	voxel get[16][16][16];
	// ...
} sector;

typedef struct {
	BYTE id;
	BYTE purity;
} biome_descriptor;

typedef struct {
	biome_descriptor biome_map[16][16];
	sector sector[8]; // list of sectors (which contain voxel data)
	BYTE sector_info; // ech bit is unset if corresponding sector is empty
	int x, z; // x and z position of chunk (in chunks)
	// ...
} chunk;

chunk* loaded_chunks[RD_DIAMETER][RD_DIAMETER];
int CHK_X, CHK_Z;
int OLD_CHK_X, OLD_CHK_Z;

chunk* generate_chunk(int _chunk_x, int _chunk_z) {
	int num_trees, tree_height, dirt_depth, gen_height;
	
	chunk* chk = calloc(1, sizeof(chunk));
	
	chk->x = _chunk_x;
	chk->z = _chunk_z;
	
	char heightmap[16][16];
	
	for (int x = 0; x < 16; x++) {
		for (int z = 0; z < 16; z++) {
			gen_height = (int) (16 * (perlin(x + chk->x * 16, z + chk->z * 16, 0.5))) + 32;
			heightmap[x][z] = gen_height + 1;
			chk->sector[(gen_height + 1) / 16].get[x][(gen_height + 1) % 16][z] = (voxel) {1};
			dirt_depth = pr_range(2, 5); // rand between 2 and 5
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
	
	num_trees = pr_range(1, 2);
	
	for (int k = 0; k < num_trees; k++) {
		int x = rand() % 12 + 2, z = rand() % 12 + 2;
		
		tree_height = rand() % 3 + 4; // rand between 4 and 6
		for (int i = tree_height; i >= 0; i--)
			chk->sector[heightmap[x][z] / 16].get[x][(heightmap[x][z] + i) % 16][z] = (voxel) {4};
		
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
		//printf("! chunk CX %i, CZ %i already exists: loading...\n", _chunk_x, _chunk_z);
		int fd = open(chunk_name, O_RDONLY);
		chk = malloc(sizeof(chunk));
		read(fd, chk, sizeof(chunk));
		close(fd);
	}
	
	return chk;
}

#endif