#ifndef __CAOIMH_H__
#define __CAOIMH_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* typedefs */
#define BYTE uint8_t
#define WORD uint16_t
#define DWORD uint32_t
#define QWORD uint64_t

#define uint8 uint8_t
#define uint16 uint16_t
#define uint32 uint32_t
#define uint64 uint64_t
#define int8 int8_t
#define int16 int16_t
#define int32 int32_t
#define int64 int64_t

#define bool unsigned char

/* macro functions */
#define SWAP_ENDIAN_DWORD(N)	\
	((N >> 24) & 0xFF) |		\
	((N << 8) & 0xFF0000) |		\
	((N >> 8) & 0xFF00) |		\
	((N << 24) & 0xFF000000)	\

#define SWAP(A, B) {			\
	A += B;						\
	B = A - B;					\
	A -= B;						\
}								\

#define SWAPF(A, B) {			\
	A ^= B;						\
	B ^= A;						\
	A ^= B;						\
}								\

#define SWAP_PTR(A, B) {		\
	void* t = A;				\
	A = B;						\
	B = t;						\
}								\

/* global variables */
char* FALLOC_POS;

/* declarations */
void falloc_init(long size);
char* falloc(int size);
void fmemcpy(char* dest, char* src, int size);

long round_byte(long _num_bits);

void rord(DWORD* set, char _n);
void rold(DWORD* set, char _n);
void rorb(BYTE* set, char _n);
void rolb(BYTE* set, char _n);

void revd(DWORD* _b);
void revb(BYTE* _b);

/* definitions */
void falloc_init(long _sz) {
	FALLOC_POS = calloc(1, _sz);
}

char* falloc(int _sz) {
	char* ptr = FALLOC_POS;
	FALLOC_POS += _sz;
	return ptr;
}

void fmemcpy(char* _dst, char* _src, int _sz) {
	for (int i = 0; i < _sz; i++) *_dst++ = *_src++;
}

long round_byte(long _num_bits) {
	while (_num_bits % 8 != 0) _num_bits++;
	return _num_bits / 8;
}

// right rotate bits in dword
void rord(DWORD* _b, char _n) {
	DWORD cr = 1;
	for (char n = _n - 1; n > 0; n--) cr |= 1 << n;
	cr &= *_b;
	*_b >>= _n;
	*_b |= cr << (32 - _n);
}

// left rotate bits in dword
void rold(DWORD* _b, char _n) {
	DWORD cr = 0;
	for (char n = _n; n > 0; n--) cr |= 1 << (32 - n);
	cr &= *_b;
	*_b <<= _n;
	*_b |= cr >> (32 - _n);
}

// right rotate bits in byte
void rorb(BYTE* _b, char _n) {
	BYTE cr = 1;
	for (char n = _n - 1; n > 0; n--) cr |= 1 << n;
	cr &= *_b;
	*_b >>= _n;
	*_b |= cr << (8 - _n);
}

// left rotate bits in byte
void rolb(BYTE* _b, char _n) {
	BYTE cr = 0;
	for (char n = _n; n > 0; n--) cr |= 1 << (8 - n);
	cr &= *_b;
	*_b <<= _n;
	*_b |= cr >> (8 - _n);
}

// reverse bits in dword
void revd(DWORD* _b) {
	DWORD cr = 0;
	while (*_b > 0) {
		cr <<= 1;
		if (*_b & 1 == 1) cr ^= 1;
		*_b >>= 1;
	}
	*_b = cr;
}

// reverse bits in byte
void revb(BYTE* _b) {
	BYTE cr = 0;
	while (*_b > 0) {
		cr <<= 1;
		if (*_b & 1 == 1) cr ^= 1;
		*_b >>= 1;
	}
	*_b = cr;
}

#endif