#ifndef __CORDAC_LINUX_FRAMEBUFFER_CORE_H__
#define __CORDAC_LINUX_FRAMEBUFFER_CORE_H__

#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>

#include <sys/ioctl.h>
#include <linux/fb.h>

typedef struct __attribute__((__packed__)) {
	uint8_t r, g, b, x;
} rgbx32;

rgbx32** sbuf = 0;

char* fb_buf = 0;
char* fb_sbuf = 0;
struct fb_fix_screeninfo fb_finfo = {0};
struct fb_var_screeninfo fb_vinfo = {0};

int fb_init(char* _fb_dev_path) {
	// open framebuffer device for reading and writing
	int fd = open(_fb_dev_path, O_RDWR | O_SYNC);
	if (fd == -1) return 1;
	
	// retrieve variable and fixed screen information
	if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_vinfo) == -1) return 2;
	if (ioctl(fd, FBIOGET_FSCREENINFO, &fb_finfo) == -1) return 2;
	
	// ensure visual part of framebuffer starts at top-left corner of screen
	fb_vinfo.xoffset = 0;
	fb_vinfo.yoffset = 0;
	if (ioctl(fd, FBIOPAN_DISPLAY, &fb_vinfo) == -1) return 3;
	
	// ensure 32 bits per pixel
	fb_vinfo.bits_per_pixel = 32;
	if (ioctl(fd, FBIOPUT_VSCREENINFO, &fb_vinfo) == -1) return 4;
	
	if (fb_vinfo.nonstd) return 5;
	
	// map framebuffer to memory and map secondary buffer
	fb_buf = (char*) mmap(0, fb_finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	fb_sbuf = (char*) mmap(0, fb_finfo.smem_len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (fb_buf == -1 || fb_sbuf == -1) return 6;
	
	if ((sbuf = malloc(fb_vinfo.yres * sizeof(void*))) == -1) return 7;
	
	for (int y = 0; y < fb_vinfo.yres; y++)
		sbuf[y] = fb_sbuf + y * fb_finfo.line_length;
	
	close(fd);
	return 0;
}

void fb_cleanup(void) {
	munmap(fb_buf, fb_finfo.smem_len);
	munmap(fb_sbuf, fb_finfo.smem_len);
}

void fb_swap(void) {
	// copy contents of secondary buffer to primary buffer
	memcpy(fb_buf, fb_sbuf, fb_finfo.smem_len);
	
	// clear secondary buffer
	for (int i = 0; i < fb_vinfo.yres; i++)
		memset(fb_sbuf + i * fb_finfo.line_length, 0, fb_vinfo.xres * 4);
}

void fb_copy(void) {
	// copy contents of secondary buffer to primary buffer
	memcpy(fb_buf, fb_sbuf, fb_finfo.smem_len);
}

#endif