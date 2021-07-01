#ifndef __BMAP_H__
#define __BMAP_H__

#include "lib/caoimh.h"
#include "lib/huffman.h"

#define __BMAPCLIB_ATTEMPT_ANYWAYS__

#define ERR_FILE_NOT_FOUND 255
#define ERR_UNKNOWN_HEADER 254
#define ERR_INVALID_PALETTE_SIZE 253
#define ERR_INCONSISTANT_HEADER_INFORMATION 252
#define ERR_EXPECTED_COLOUR_PALETTE_NOT_PRESENT 251
#define ERR_CONFLICTING_HEADER_INFORMATION 250

#define LCS_CALIBRATED_RGB 0x00000000
#define LCS_SRGB 0x73524742
#define GM_ABS_COLOURMETRIC 0x00000008
#define GM_BUISNESS 0x00000001
#define GM_GRAPHICS 0x00000002
#define GM_IMAGE 0x00000004

#define UNKNOWN 0
#define BITMAPV5HEADER 1
#define BITMAPV4HEADER 2
#define BITMAPINFOHEADER 3
#define BITMAPCOREHEADER 4
#define OS22XBITMAPHEADER 5
#define BITMAPCOREHEADER2 5

#define BI_RGB 0
#define BI_RLE8 1
#define BI_RLE4 2
#define BI_BITFIELDS 3
#define BI_JPEG 4
#define BI_PNG 5
#define BI_ALPHABITFIELDS 6

unsigned static char ERROR_CODE = 0;

struct rgb_pixel_24 {
	BYTE r, g, b;
} __attribute__ ((__packed__));

struct rgba_pixel_8 {
	BYTE r : 2;
	BYTE g : 2;
	BYTE b : 2;
	BYTE a : 2;
} __attribute__ ((__packed__));

struct rgba_pixel_16 {
	BYTE r : 4;
	BYTE g : 4;
	BYTE b : 4;
	BYTE a : 4;
} __attribute__ ((__packed__));

struct rgba_pixel_32 {
	BYTE r, g, b, a;
} __attribute__ ((__packed__));

struct CIEXYZ {
	DWORD x, y, z; // point in CIE colour space
} __attribute__ ((__packed__));

struct CIEXYZTRIPLE {
	struct CIEXYZ red, green, blue; // endpoints for red green and blue in CIE colour space
} __attribute__ ((__packed__));

struct BM_BITMAPFILEHEADER {
	WORD field; // signature of file type (0x42 0x4D for bitmap)
	DWORD filesize; // size of entire file (in bytes)
	DWORD nullspace; // unused / aplication specific
	DWORD offset; // offset (in bytes) to image data
} __attribute__ ((__packed__));

struct BM_BITMAPCOREHEADER {
	DWORD length; // size of bitmap header (12 bytes) - starts here
	DWORD width; // image width (pixels)
	DWORD height; // image height (pixels)
	WORD num_planes; // number of colour planes (must be 1)
	WORD bpp; // number of bits per pixel (colour depth)
} __attribute__ ((__packed__));

struct BM_BITMAPINFOHEADER {
	DWORD length; // size of bitmap header (40 bytes) - starts here
	DWORD width; // image width (pixels)
	DWORD height; // image height (pixels)
	WORD num_planes; // number of colour planes (must be 1)
	WORD bpp; // number of bits per pixel (colour depth)
	DWORD compression_method; // most often BI_RGB (0 - no compression)
	DWORD image_size; // size of padded bitmap data (can be 0 if BI_RGB is used)
	DWORD horizontal_resolution; // horizontal resolution of image in pixels per metre
	DWORD vertical_resolution; // vertical resolution of image in pixels per metre
	DWORD num_colours; // number of colours in the colour palette (0 defaults to 2^n)
	DWORD num_important_colours; // number of important colours in colour palette (0 if all colours are important)
} __attribute__ ((__packed__));

struct BM_OS22XBITMAPHEADER {
	DWORD length; // size of bitmap header (64 bytes) - starts here
	DWORD width; // image width (pixels)
	DWORD height; // image height (pixels)
	WORD num_planes; // number of colour planes (must be 1)
	WORD bpp; // number of bits per pixel (colour depth)
	DWORD compression_method; // most often BI_RGB (0 - no compression)
	DWORD image_size; // can be 0 if BI_RGB (0) is used
	DWORD horizontal_resolution; // horizontal resolution of image
	DWORD vertical_resolution; // vertical resolution of image
	DWORD num_colours; // number of colours in the colour palette (0 defaults to 2^n)
	DWORD num_important_colours; // number of important colours in colour palette (0 if all colours are important)
	WORD resolution_units; // units of horizontal and vertical resolutions (0 is pixels per metre)
	WORD padding; // padding - should be 0
	WORD paint_direction; // how the bitmap is drawn (0 means origin at bottom-left and fill from right-left then bottom-top)
	WORD halftoning_method; // specifies the halftoning algorithm to be used when drawing the image (most often 0 - no hlaftoning)
	DWORD halftoning_parameter_a; // error damping percentage for halftoning method of 1 and x dimension of haftoning pattern otherwise
	DWORD halftoning_parameter_b; // used as y dimension of halfoning pattern for halftoning methods other than 1 or 0, unused otherwise
	DWORD colour_coding; // colour coding used in colour palette (always 0 for RGB)
	DWORD nullspace; // unused / aplication specific
} __attribute__ ((__packed__));

struct BM_BITMAPV4HEADER {
	DWORD length; // size of bitmap header (108 bytes) - starts here
	DWORD width; // image width (pixels)
	DWORD height; // image height (pixels)
	WORD num_planes; // number of colour planes (must be 1)
	WORD bpp; // number of bits per pixel (colour depth)
	DWORD compression_method; // most often BI_RGB (0 - no compression)
	DWORD image_size; // size of padded bitmap data (can be 0 if BI_RGB is used)
	DWORD horizontal_resolution; // horizontal resolution of image in pixels per metre
	DWORD vertical_resolution; // vertical resolution of image in pixels per metre
	DWORD num_colours; // number of colours in the colour palette (0 defaults to 2^n)
	DWORD num_important_colours; // number of important colours in colour palette (0 if all colours are important)
	DWORD red_mask; // red bit mask
	DWORD green_mask; // green bit mask
	DWORD blue_mask; // blue bit mask
	DWORD alpha_mask; // alpha bit-mask
	DWORD colour_space; // the colour space used (default is LCS_CALIBRATED_RGB (0) - use given endpoints and gamma values)
	struct CIEXYZTRIPLE endpoints; // used if LCS_CALIBRATED_RGB (0) is used
	DWORD gamma_red; // response curve for red - first 2 bytes are integer value and lsat two are fractional portion
	DWORD gamma_green; // response curve for green - first 2 bytes are integer value and lsat two are fractional portion
	DWORD gamma_blue; // response curve for blue - first 2 bytes are integer value and lsat two are fractional portion
} __attribute__ ((__packed__));

struct BM_BITMAPV5HEADER {
	DWORD length; // size of bitmap header (124 bytes) - starts here
	DWORD width; // image width (pixels)
	DWORD height; // image height (pixels)
	WORD num_planes; // number of colour planes (must be 1)
	WORD bpp; // number of bits per pixel (colour depth)
	DWORD compression_method; // most often BI_RGB (0 - no compression)
	DWORD image_size; // size of padded bitmap data (can be 0 if BI_RGB is used)
	DWORD horizontal_resolution; // horizontal resolution of image in pixels per metre
	DWORD vertical_resolution; // vertical resolution of image in pixels per metre
	DWORD num_colours; // number of colours in the colour palette (0 defaults to 2^n)
	DWORD num_important_colours; // number of important colours in colour palette (0 if all colours are important)
	DWORD red_mask; // red bit mask
	DWORD green_mask; // green bit mask
	DWORD blue_mask; // blue bit mask
	DWORD alpha_mask; // alpha bit-mask
	DWORD colour_space; // the colour space used (default is LCS_CALIBRATED_RGB (0) - use given endpoints and gamma values)
	struct CIEXYZTRIPLE endpoints; // used if LCS_CALIBRATED_RGB (0) is used
	DWORD gamma_red; // response curve for red - first 2 bytes are integer value and lsat two are fractional portion
	DWORD gamma_green; // response curve for green - first 2 bytes are integer value and lsat two are fractional portion
	DWORD gamma_blue; // response curve for blue - first 2 bytes are integer value and lsat two are fractional portion
	DWORD intent; // gamut mapping intent of image
} __attribute__ ((__packed__));

struct BM_EXPORTDATA {
	long bit_length; // length of unpadded image data in bits
	long bit_width; // width of unpadded image data in bits
	long padded_length; // length of padded image data in bytes
	long padded_width; // width of padded image data in bytes
	BYTE header_format : 3; // type of bitmap header used (0 = unknown, 1 = BITMAPV5HEADER, 2 = BITMAPV4HEADER, 3 = BITMAPINFOHEADER, 4 = BITMAPCOREHEADER, 5 = OS22XBITMAPHEADER)
	BYTE padding : 4; // number of padded bits per row (each row of image must be a multiple of 4 bytes)
	BYTE padded : 1; // 1 if data is padded, 0 if data is not padded
	BYTE NO_PALETTE; // set if no colour palette is used
};

typedef struct bitmap {
	struct BM_BITMAPFILEHEADER file_header;
	struct BM_BITMAPV5HEADER info;
	struct BM_EXPORTDATA extra;
	struct rgba_pixel_32* palette;
	struct rgba_pixel_32** data;
	BYTE* rdata;
	long plen; // length of unpadded image data in pixels
} bitmap;

typedef struct png {
	int a;
} png;

void export_bitmap(char* filepath, bitmap _b);
bitmap import_bitmap(char* filepath);

void bitmap8_to_32rgba(bitmap* _b);
void bitmap16_to_32rgba(bitmap* _b);
void bitmap24_to_32rgba(bitmap* _b);
void bitmap_expand_from_colour_table(bitmap* _b);

#endif