#ifndef __MCN_LINUX_MOUSE_CORE_H__
#define __MCN_LINUX_MOUSE_CORE_H__

#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include <linux/input.h>

static int g_mdev_fd;

int mouse_init(char* _fp) {
	g_mdev_fd = open(_fp, O_RDONLY | O_NONBLOCK);
	if (g_mdev_fd == -1) return 1;
	return 0;
}

typedef struct {
	uint8_t lbtn : 1;
	uint8_t rbtn : 1;
	uint8_t mbtn : 1;
	int8_t rel_x;
	int8_t rel_y;
} mouse_event;

mouse_event get_mouse_event(void) {
	mouse_event ev = {0};
	read(g_mdev_fd, &ev, sizeof(ev));
	return ev;
}

#endif