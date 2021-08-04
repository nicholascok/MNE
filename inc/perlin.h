#ifndef __MCN_PERLIN_H__
#define __MCN_PERLIN_H__

// perlin noise with options for multiple octaves, persistance, and lacunarity.

#include <stdint.h>

#include "prng.h"
#include "cmath.h"

struct {
	double lacunarity;
	double persistance;
	uint8_t octaves;
	uint8_t table[512];
} plc;


// return gradient vector based on random hash
double __perlin_grad(uint8_t _h, double _x, double _y, double _z) {
	switch (_h & 0x0F) {
		case 0x00: return  _x + _y; // grad vec (1, 1, 0)
		case 0x01: return  _y - _x; // grad vec (-1, 1, 0)
		case 0x02: return  _x - _y; // grad vec (1, -1, 0)
		case 0x03: return -_x - _y; // grad vec (-1, -1, 0)
		case 0x04: return  _x + _z; // grad vec (1, 0, 1)
		case 0x05: return  _x - _z; // grad vec (1, 0, -1)
		case 0x06: return  _z - _x; // grad vec (-1, 0, 1)
		case 0x07: return -_x - _z; // grad vec (-1, 0, -1)
		case 0x08: return  _y + _z; // grad vec (0, 1, 1)
		case 0x09: return  _z - _y; // grad vec (0, -1, 1)
		case 0x0A: return  _y - _z; // grad vec (0, 1, -1)
		case 0x0B: return -_y - _z; // grad vec (0, -1, -1)
		case 0x0C: return  _x + _y; // grad vec (1, 1, 0)
		case 0x0D: return  _y - _x; // grad vec (-1, 1, 0)
		case 0x0E: return  _z - _y; // grad vec (0, -1, 1)
		case 0x0F: return -_y - _z; // grad vec (0, -1, -1)
		default: return 0;
	}
}

// lineraly interpolate _v on the interval [0.0, 1.0] to a value on the interval [_0, _1]
double __perlin_lintp(double _v, double _0, double _1) {
	return _v * (_1 - _0) + _0;
}

// fade function (sigmoid approximation)
double __perlin_fade(double _t) {
	return _t * _t * _t * (_t * (6 * _t - 15) + 10);
}

void pl_init(uint32_t _seed, uint8_t _octaves, double _lacunarity, double _persistance) {
	plc.lacunarity = _lacunarity;
	plc.persistance = _persistance;
	plc.octaves = _octaves;
	
	// set seed for PRNG
	if (!prc.seed[0]) pr_init(_seed);
	else pr_set_seed(_seed);
	
	// setup permutation table
	for (int i = 0; i < 256; i++)
		plc.table[i] = i;
	
	// randomly swap values in permutation table to form a random distribution
	for (int r, i = 0; i < 256; i++) {
		// generate random index
		r = pr_range(0, 256);
		
		// swap table values at i and r
		plc.table[i] ^= plc.table[r];
		plc.table[r] ^= plc.table[i];
		plc.table[i] ^= plc.table[r];
	}
	
	// copy first half of table into secord to permit higher indexing
	for (int i = 0; i < 256; i++)
		plc.table[256 + i] = plc.table[i];
}

double perlin(double _x, double _y, double _z) {
	_x /= 16, _y /= 16, _z /= 16;
	double divisor = 0;
	double factor = 1;
	double result = 0;
	
	for (int i = 0; i < plc.octaves; i++) {
		// find grid points in neighbourhood of point (x, y, z)
		// & with 256 same as mod with 256 (make sure coordinates are in bounds)
		int x = IFLOOR(_x) & 255;
		int y = IFLOOR(_y) & 255;
		int z = IFLOOR(_z) & 255;
		
		double X = _x - IFLOOR(_x);
		double Y = _y - IFLOOR(_y);
		double Z = _z - IFLOOR(_z);
		
		// smooth result by putting points throush a fade function (quintic function - 6x^5 - 15x^4 + 10x^3)
		double u = __perlin_fade(X);
		double v = __perlin_fade(Y);
		double w = __perlin_fade(Z);
		
		int A = plc.table[x    ] + y, AA = plc.table[A] + z, AB = plc.table[A + 1] + z;
		int B = plc.table[x + 1] + y, BA = plc.table[B] + z, BB = plc.table[B + 1] + z;
		
		
		// compute dot product of offset vectors with random gradient vectors (picked out of a set
		// of twelve unique based on PRNG using the permutation table)
		result += factor *
		__perlin_lintp(w, __perlin_lintp(v, __perlin_lintp(u, __perlin_grad(plc.table[AA    ], X    , Y    , Z    ),
															  __perlin_grad(plc.table[BA    ], X - 1, Y    , Z    )),
											__perlin_lintp(u, __perlin_grad(plc.table[AB    ], X    , Y - 1, Z    ),
															  __perlin_grad(plc.table[BB    ], X - 1, Y - 1, Z    ))),
						  __perlin_lintp(v, __perlin_lintp(u, __perlin_grad(plc.table[AA + 1], X    , Y    , Z - 1),
															  __perlin_grad(plc.table[BA + 1], X - 1, Y    , Z - 1)),
											__perlin_lintp(u, __perlin_grad(plc.table[AB + 1], X    , Y - 1, Z - 1),
															  __perlin_grad(plc.table[BB + 1], X - 1, Y - 1, Z - 1))));
		
		divisor += factor;
		factor *= plc.persistance;
		
		_x *= plc.lacunarity;
		_y *= plc.lacunarity;
		_z *= plc.lacunarity;
	}
	
	result /= divisor;
	
	return result;
}

#endif