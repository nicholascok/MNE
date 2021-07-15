#ifndef __MCN_ACORN_PRNG_H__
#define __MCN_ACORN_PRNG_H__

// implementation of the ACORN algorithm for seeded pseudo-random numbers
// base off of description provided at https://web.archive.org/web/20050824081746/http://www.siam.org/siamnews/11-00/splitting.pdf
// and from the author's website at http://acorn.wikramaratna.org/

#include <stdint.h>

#define PR_ORDER 120

struct {
	uint64_t mod;
	uint64_t seed[PR_ORDER];
} prc;

void pr_set_seed(uint32_t _seed) {
	// update seed
	prc.seed[0] = _seed;
	for (short i = 1; i < PR_ORDER; i++) {
		// generate values from initial seed using a Park-Miller RNG
		prc.seed[i] = prc.seed[i - 1] * (uint64_t) 48271 % 0x7FFFFFFFFFFFFFFF;
		// ensure initial values are odd in order to ensure
		// they are relatively prime with the modulus (a power of 2)
		if (!(prc.seed[i] & 1)) prc.seed[i]++;
	}
}

void pr_init(uint32_t _seed) {
	prc.mod = ((uint64_t) 1 << 60) - 1; // 2^60 - 1
	prc.seed[0] = _seed;
	for (short i = 1; i < PR_ORDER; i++) {
		// generate values from initial seed using a Park-Miller RNG
		prc.seed[i] = prc.seed[i - 1] * (uint64_t) 48271 % 0x7FFFFFFFFFFFFFFF;
		// ensure initial values are odd in order to ensure
		// they are relatively prime with the modulus (a power of 2)
		if (!(prc.seed[i] & 1)) prc.seed[i]++;
	}
}

int pr_range(int _min, int _max) {
	// generate next random value
	for (short i = 1; i < PR_ORDER; i++) {
		// note: x % 2^n = x & (2^n - 1), and since our modulus is 2^60,
		// prc.mod is set to 2^60 - 1
		prc.seed[i] = (prc.seed[i - 1] + prc.seed[i]) & prc.mod;
	}
	
	// note: prc.mod = modulus - 1 -> modulus = prc.mod + 1
	// divide by modulus to normalise result - we will get values
	// on the interval [0, 1]
	return prc.seed[PR_ORDER - 1] % (_max - _min) + _min;
}

double pr_next_integer(void) {
	// generate next random value
	for (short i = 1; i < PR_ORDER; i++) {
		// note: x % 2^n = x & (2^n - 1), and since our modulus is 2^60,
		// prc.mod is set to 2^60 - 1
		prc.seed[i] = (prc.seed[i - 1] + prc.seed[i]) & prc.mod;
	}
	
	// note: prc.mod = modulus - 1 -> modulus = prc.mod + 1
	// divide by modulus to normalise result - we will get values
	// on the interval [0, 1]
	return prc.seed[PR_ORDER - 1];
}

double pr_next_double(void) {
	// generate next random value
	for (short i = 1; i < PR_ORDER; i++) {
		// note: x % 2^n = x & (2^n - 1), and since our modulus is 2^60,
		// prc.mod is set to 2^60 - 1
		prc.seed[i] = (prc.seed[i - 1] + prc.seed[i]) & prc.mod;
	}
	
	// note: prc.mod = modulus - 1 -> modulus = prc.mod + 1
	// divide by modulus to normalise result - we will get values
	// on the interval [0, 1]
	return (double) prc.seed[PR_ORDER - 1] / (prc.mod + 1);
}

#endif