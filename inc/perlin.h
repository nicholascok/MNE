#ifndef __CORDAC_PERLIN_H__
#define __CORDAC_PERLIN_H__

// perlin noise with options for multiple octaves, persistance, and lacunarity.

#include <stdint.h>

#include "prng.h"

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

void pl_init(uint32_t _seed, uint8_t _octaves, double _lacunarity, double _persistance) {
	plc.lacunarity = _lacunarity;
	plc.persistance = _persistance;
	plc.octaves = _octaves;
	
	// set seed for PRNG
	if (!prc.seed) pr_init(_seed);
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
	double max_value = 0;
	double result = 0;
	double factor = 1;
	double scale = 0.8;
	
	for (int oc = 0; oc < plc.octaves; oc++) {
		// find grid points in neighbourhood of point (x, y, z)
		// & with 256 same as mod with 256 (make sure coordinates are in bounds)
		unsigned x0 = (int) (_x * scale) - (_x * scale < 0), x1 = (x0 + 1);
		unsigned y0 = (int) (_y * scale) - (_y * scale < 0), y1 = (y0 + 1);
		unsigned z0 = (int) (_z * scale) - (_z * scale < 0), z1 = (z0 + 1);
		
		// relative coordinates of point in interval [0.0, 1.0]
		double rx0 = (_x * scale) - (double) x0, rx1 = rx0 - 1;
		double ry0 = (_y * scale) - (double) y0, ry1 = ry0 - 1;
		double rz0 = (_z * scale) - (double) z0, rz1 = rz0 - 1;
		
		// make noise wrap by ensuring all values can index the permutation table from 0-255
		x0 = x0 & 256;
		y0 = y0 & 256;
		z0 = z0 & 256;
		
		// smooth result by putting points throush a fade function (quintic function - 6x^5 - 15x^4 + 10x^3)
		double u = rx0 * rx0 * rx0 * (rx0 * (6 * rx0 - 15) + 10);
		double v = ry0 * ry0 * ry0 * (ry0 * (6 * ry0 - 15) + 10);
		double w = rz0 * rz0 * rz0 * (rz0 * (6 * rz0 - 15) + 10);
		
		// compute dot product of offset vectors with random gradient vectors (picked out of a set
		// of twelve unique based on PRNG using the permutation table)
		double gdot000 = __perlin_grad(plc.table[plc.table[plc.table[x0] + y0] + z0], rx0, ry0, rz0);
		double gdot001 = __perlin_grad(plc.table[plc.table[plc.table[x0] + y0] + z1], rx0, ry0, rz1);
		double gdot010 = __perlin_grad(plc.table[plc.table[plc.table[x0] + y1] + z0], rx0, ry1, rz0);
		double gdot011 = __perlin_grad(plc.table[plc.table[plc.table[x0] + y1] + z1], rx0, ry1, rz1);
		double gdot100 = __perlin_grad(plc.table[plc.table[plc.table[x1] + y0] + z0], rx1, ry0, rz0);
		double gdot101 = __perlin_grad(plc.table[plc.table[plc.table[x1] + y0] + z1], rx1, ry0, rz1);
		double gdot110 = __perlin_grad(plc.table[plc.table[plc.table[x1] + y1] + z0], rx1, ry1, rz0);
		double gdot111 = __perlin_grad(plc.table[plc.table[plc.table[x1] + y1] + z1], rx1, ry1, rz1);
		
		double k0 = gdot000;
		double k1 = gdot100 - gdot000;
		double k2 = gdot010 - gdot000;
		double k3 = gdot001 - gdot000;
		double k4 = gdot000 + gdot110 - gdot100 - gdot010;
		double k5 = gdot000 + gdot101 - gdot100 - gdot001;
		double k6 = gdot000 + gdot011 - gdot010 - gdot001;
		double k7 = gdot100 + gdot010 + gdot001 + gdot111 - gdot000 - gdot110 - gdot101 - gdot011;
		
		result += factor * (k0 + k1 * u + k2 * v + k3 * w + k4 * u * v + k5 * u * w + k6 * v * w + k7 * u * v * w);
		
		// after each pass, multiply the point in the next pass by the scale (lacunarity)
		// and multiply the result by the persistance (usually less than 1.0)
		factor *= plc.persistance;
		scale *= plc.lacunarity;
	}
	
	result /= plc.octaves;
	result /= 4;
	
	return 0.5 * result / sqrt(1 + result * result) + 0.5;
}

#endif