#ifndef __CORDAC_LINUX_KEYBOARD_CORE_H__
#define __CORDAC_LINUX_KEYBOARD_CORE_H__

#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include <linux/input.h>

#define KEY_RELEASE 0x00
#define KEY_PRESS	0x01
#define KEY_REPEAT	0x02

#define MAX_OVER_READ 16

extern void* memset(void*, int, size_t);
extern int ioctl(int, unsigned long int, ...) __THROW;

static int g_kdev_fd;

// used to store keymap (bit array containing state of each key)
static uint8_t key_map[KEY_MAX / 8 + 1];

int keyboard_init(char* _fp, int _extra_flags) {
	g_kdev_fd = open(_fp, O_RDONLY | _extra_flags);
	if (g_kdev_fd == -1) return 1;
	return 0;
}

typedef struct {
	struct timeval time;
	uint16_t __type;
	uint16_t key;
	uint32_t action;
} key_event;

// converts key codes from a USB keyboard to ASCII chars when possible
// currently no support for PS/2 as model specific scan code conversion is necessary
// (note: as a result, some keys are remapped)
uint16_t __key_to_ascii(uint16_t _c) {
	switch (_c) {
		case KEY_RESERVED			: return 0x00 ;
		case KEY_ESC				: return 0x1B ;
		case KEY_1					: return 0x31 ;
		case KEY_2					: return 0x32 ;
		case KEY_3					: return 0x33 ;
		case KEY_4					: return 0x34 ;
		case KEY_5					: return 0x35 ;
		case KEY_6					: return 0x36 ;
		case KEY_7					: return 0x37 ;
		case KEY_8					: return 0x38 ;
		case KEY_9					: return 0x39 ;
		case KEY_0					: return 0x30 ;
		case KEY_MINUS				: return 0x2D ;
		case KEY_EQUAL				: return 0x3D ;
		case KEY_BACKSPACE			: return 0x08 ;
		case KEY_TAB				: return 0x09 ;
		case KEY_Q					: return 0x71 ;
		case KEY_W					: return 0x77 ;
		case KEY_E					: return 0x65 ;
		case KEY_R					: return 0x72 ;
		case KEY_T					: return 0x74 ;
		case KEY_Y					: return 0x79 ;
		case KEY_U					: return 0x75 ;
		case KEY_I					: return 0x69 ;
		case KEY_O					: return 0x6F ;
		case KEY_P					: return 0x70 ;
		case KEY_LEFTBRACE			: return 0x5D ;
		case KEY_RIGHTBRACE			: return 0x5B ;
		case KEY_ENTER				: return 0x0A ;
		case KEY_A					: return 0x61 ;
		case KEY_S					: return 0x73 ;
		case KEY_D					: return 0x64 ;
		case KEY_F					: return 0x66 ;
		case KEY_G					: return 0x67 ;
		case KEY_H					: return 0x68 ;
		case KEY_J					: return 0x6A ;
		case KEY_K					: return 0x6B ;
		case KEY_L					: return 0x6C ;
		case KEY_SEMICOLON			: return 0x3B ;
		case KEY_APOSTROPHE			: return 0x27 ;
		case KEY_GRAVE				: return 0x60 ;
		case KEY_BACKSLASH			: return 0x5C ;
		case KEY_Z					: return 0x7A ;
		case KEY_X					: return 0x78 ;
		case KEY_C					: return 0x63 ;
		case KEY_V					: return 0x76 ;
		case KEY_B					: return 0x62 ;
		case KEY_N					: return 0x6E ;
		case KEY_M					: return 0x6D ;
		case KEY_COMMA				: return 0x2C ;
		case KEY_DOT				: return 0x2E ;
		case KEY_SLASH				: return 0x2F ;
		case KEY_SPACE				: return 0x20 ;
		// REMAPPINGS:
		case KEY_LEFTCTRL			: return 0x300;
		case KEY_LEFTSHIFT			: return 0x301;
		case KEY_RIGHTSHIFT			: return 0x302;
		case KEY_KPASTERISK			: return 0x303;
		case KEY_LEFTALT			: return 0x304;
		case KEY_CAPSLOCK			: return 0x305;
		case KEY_F1					: return 0x306;
		case KEY_F2					: return 0x307;
		case KEY_F3					: return 0x308;
		case KEY_F4					: return 0x309;
		case KEY_F5					: return 0x30A;
		case KEY_F6					: return 0x30B;
		case KEY_F7					: return 0x30C;
		case KEY_F8					: return 0x30D;
		case KEY_F9					: return 0x30E;
		case KEY_F10				: return 0x30F;
		case KEY_NUMLOCK			: return 0x310;
		case KEY_SCROLLLOCK			: return 0x311;
		case KEY_KP7				: return 0x312;
		case KEY_KP8				: return 0x313;
		case KEY_KP9				: return 0x314;
		case KEY_KPMINUS			: return 0x315;
		case KEY_KP4				: return 0x316;
		case KEY_KP5				: return 0x317;
		case KEY_KP6				: return 0x318;
		case KEY_KPPLUS				: return 0x319;
		case KEY_KP1				: return 0x31A;
		case KEY_KP2				: return 0x31B;
		case KEY_KP3				: return 0x31C;
		case KEY_KP0				: return 0x31D;
		case KEY_KPDOT				: return 0x31E;
		case KEY_ZENKAKUHANKAKU		: return 0x31F;
		case KEY_102ND				: return 0x320;
		case KEY_F11				: return 0x321;
		case KEY_F12				: return 0x322;
		case KEY_RO					: return 0x323;
		case KEY_KATAKANA			: return 0x324;
		case KEY_HIRAGANA			: return 0x325;
		case KEY_HENKAN				: return 0x326;
		case KEY_KATAKANAHIRAGANA	: return 0x327;
		case KEY_MUHENKAN			: return 0x328;
		case KEY_KPJPCOMMA			: return 0x329;
		case KEY_KPENTER			: return 0x32A;
		case KEY_RIGHTCTRL			: return 0x32B;
		case KEY_KPSLASH			: return 0x32C;
		case KEY_SYSRQ				: return 0x32D;
		case KEY_RIGHTALT			: return 0x32E;
		case KEY_LINEFEED			: return 0x32F;
		case KEY_HOME				: return 0x330;
		case KEY_UP					: return 0x331;
		case KEY_PAGEUP				: return 0x332;
		case KEY_LEFT				: return 0x333;
		case KEY_RIGHT				: return 0x334;
		case KEY_END				: return 0x335;
		case KEY_DOWN				: return 0x336;
		case KEY_PAGEDOWN			: return 0x337;
		case KEY_INSERT				: return 0x338;
		case KEY_DELETE				: return 0x339;
		case KEY_MACRO				: return 0x33A;
		case KEY_MUTE				: return 0x33B;
		case KEY_VOLUMEDOWN			: return 0x33C;
		case KEY_VOLUMEUP			: return 0x33D;
		case KEY_POWER				: return 0x33E;
		case KEY_KPEQUAL			: return 0x33F;
		case KEY_KPPLUSMINUS		: return 0x340;
		case KEY_PAUSE				: return 0x341;
		case KEY_SCALE				: return 0x342;
		case KEY_KPCOMMA			: return 0x343;
		case KEY_HANGEUL			: return 0x344;
		case KEY_HANJA				: return 0x345;
		case KEY_YEN				: return 0x346;
		case KEY_LEFTMETA			: return 0x347;
		case KEY_RIGHTMETA			: return 0x348;
		case KEY_COMPOSE			: return 0x349;
		case KEY_STOP				: return 0x34A;
		case KEY_AGAIN				: return 0x34B;
		case KEY_PROPS				: return 0x34C;
		case KEY_UNDO				: return 0x34D;
		case KEY_FRONT				: return 0x34E;
		case KEY_COPY				: return 0x34F;
		case KEY_OPEN				: return 0x350;
		case KEY_PASTE				: return 0x351;
		case KEY_FIND				: return 0x352;
		case KEY_CUT				: return 0x353;
		case KEY_HELP				: return 0x354;
		case KEY_MENU				: return 0x355;
		case KEY_CALC				: return 0x356;
		case KEY_SETUP				: return 0x357;
		case KEY_SLEEP				: return 0x358;
		case KEY_WAKEUP				: return 0x359;
		case KEY_FILE				: return 0x35A;
		case KEY_SENDFILE			: return 0x35B;
		case KEY_DELETEFILE			: return 0x35C;
		case KEY_XFER				: return 0x35D;
		case KEY_PROG1				: return 0x35E;
		case KEY_PROG2				: return 0x35F;
		case KEY_WWW				: return 0x360;
		case KEY_MSDOS				: return 0x361;
		case KEY_COFFEE				: return 0x362;
		case KEY_ROTATE_DISPLAY		: return 0x363;
		case KEY_CYCLEWINDOWS		: return 0x364;
		case KEY_MAIL				: return 0x365;
		case KEY_BOOKMARKS			: return 0x366;
		case KEY_COMPUTER			: return 0x367;
		case KEY_BACK				: return 0x368;
		case KEY_FORWARD			: return 0x369;
		case KEY_CLOSECD			: return 0x36A;
		case KEY_EJECTCD			: return 0x36B;
		case KEY_EJECTCLOSECD		: return 0x36C;
		case KEY_NEXTSONG			: return 0x36D;
		case KEY_PLAYPAUSE			: return 0x36E;
		case KEY_PREVIOUSSONG		: return 0x36F;
		case KEY_STOPCD				: return 0x370;
		case KEY_RECORD				: return 0x371;
		case KEY_REWIND				: return 0x372;
		case KEY_PHONE				: return 0x373;
		case KEY_ISO				: return 0x374;
		case KEY_CONFIG				: return 0x375;
		case KEY_HOMEPAGE			: return 0x376;
		case KEY_REFRESH			: return 0x377;
		case KEY_EXIT				: return 0x378;
		case KEY_MOVE				: return 0x379;
		case KEY_EDIT				: return 0x37A;
		case KEY_SCROLLUP			: return 0x37B;
		case KEY_SCROLLDOWN			: return 0x37C;
		case KEY_KPLEFTPAREN		: return 0x37D;
		case KEY_KPRIGHTPAREN		: return 0x37E;
		case KEY_NEW				: return 0x37F;
		case KEY_REDO				: return 0x380;
		case KEY_F13				: return 0x381;
		case KEY_F14				: return 0x382;
		case KEY_F15				: return 0x383;
		case KEY_F16				: return 0x384;
		case KEY_F17				: return 0x385;
		case KEY_F18				: return 0x386;
		case KEY_F19				: return 0x387;
		case KEY_F20				: return 0x388;
		case KEY_F21				: return 0x389;
		case KEY_F22				: return 0x38A;
		case KEY_F23				: return 0x38B;
		case KEY_F24				: return 0x38C;
		case KEY_PLAYCD				: return 0x38D;
		case KEY_PAUSECD			: return 0x38E;
		case KEY_PROG3				: return 0x38F;
		case KEY_PROG4				: return 0x390;
		case KEY_DASHBOARD			: return 0x391;
		case KEY_SUSPEND			: return 0x392;
		case KEY_CLOSE				: return 0x393;
		case KEY_PLAY				: return 0x394;
		case KEY_FASTFORWARD		: return 0x395;
		case KEY_BASSBOOST			: return 0x396;
		case KEY_PRINT				: return 0x397;
		case KEY_HP					: return 0x398;
		case KEY_CAMERA				: return 0x399;
		case KEY_SOUND				: return 0x39A;
		case KEY_QUESTION			: return 0x39B;
		case KEY_EMAIL				: return 0x39C;
		case KEY_CHAT				: return 0x39D;
		case KEY_SEARCH				: return 0x39E;
		case KEY_CONNECT			: return 0x39F;
		case KEY_FINANCE			: return 0x3A0;
		case KEY_SPORT				: return 0x3A1;
		case KEY_SHOP				: return 0x3A2;
		case KEY_ALTERASE			: return 0x3A3;
		case KEY_CANCEL				: return 0x3A4;
		case KEY_BRIGHTNESSDOWN		: return 0x3A5;
		case KEY_BRIGHTNESSUP		: return 0x3A6;
		case KEY_MEDIA				: return 0x3A7;
		case KEY_SWITCHVIDEOMODE	: return 0x3A8;
		case KEY_KBDILLUMTOGGLE		: return 0x3A9;
		case KEY_KBDILLUMDOWN		: return 0x3AA;
		case KEY_KBDILLUMUP			: return 0x3AB;
		case KEY_SEND				: return 0x3AC;
		case KEY_REPLY				: return 0x3AD;
		case KEY_FORWARDMAIL		: return 0x3AE;
		case KEY_SAVE				: return 0x3AF;
		case KEY_DOCUMENTS			: return 0x3B0;
		case KEY_BATTERY			: return 0x3B1;
		case KEY_BLUETOOTH			: return 0x3B2;
		case KEY_WLAN				: return 0x3B3;
		case KEY_UWB				: return 0x3B4;
		case KEY_UNKNOWN			: return 0x3B5;
		case KEY_VIDEO_NEXT			: return 0x3B6;
		case KEY_VIDEO_PREV			: return 0x3B7;
		case KEY_BRIGHTNESS_CYCLE	: return 0x3B8;
		case KEY_BRIGHTNESS_AUTO	: return 0x3B9;
		case KEY_DISPLAY_OFF		: return 0x3BA;
		case KEY_WWAN				: return 0x3BB;
		case KEY_RFKILL				: return 0x3BC;
		case KEY_MICMUTE			: return 0x3BD;
		default: return _c;
	}
}

// get raw key code from device
key_event get_key_event_raw(void) {
	key_event ev = {0};
	read(g_kdev_fd, &ev, sizeof(ev));
	if (ev.__type == 1) return ev;
	return (key_event) {ev.time, 0, 0, -1};
}

// same as before but returns ANSI chars when possible
key_event get_key_event(void) {
	key_event ev = {0};
	read(g_kdev_fd, &ev, sizeof(ev));
	ev.key = __key_to_ascii(ev.key);
	if (ev.__type == 1) return ev;
	return (key_event) {ev.time, 0, 0, -1};
}

// same as before, but over-reads to avoid buffering
// (the maximum number of keys read is MAX_OVER_READ)
key_event get_key_event_raw_no_buffer(void) {
	key_event ev[MAX_OVER_READ] = {0};
	read(g_kdev_fd, ev, sizeof(ev));
	if (ev[1].__type == 1) return ev[1];
	return (key_event) {ev[1].time, 0, 0, -1};
}

key_event get_key_event_no_buffer(void) {
	key_event ev[MAX_OVER_READ] = {0};
	read(g_kdev_fd, ev, sizeof(ev));
	ev[1].key = __key_to_ascii(ev[1].key);
	if (ev[1].__type == 1) return ev[1];
	return (key_event) {ev[1].time, 0, 0, -1};
}

int sync_keymap(void) {
	memset(key_map, 0x00, sizeof(key_map));
	if (ioctl(g_kdev_fd, EVIOCGKEY(sizeof(key_map)), key_map) < 0)
		return -1;
	return 0;
}

// sync keymap and check state of key
int check_key(uint16_t _k) {
	if (sync_keymap() < 0) return -1;
	return !!(key_map[_k / 8] & (1 << (_k % 8)));
}

// do not sync keymap and check state of key
int check_key_local(uint16_t _k) {
	return !!(key_map[_k / 8] & (1 << (_k % 8)));
}

// copies at most _n active key codes into buffer at _buf
int add_keys_to_buffer(uint16_t* _buf, int _n) {
	for (int k = 0, i = 0; k < KEY_CNT && i < _n; k++)
		if (check_key_local(k)) _buf[i++] = k;
	return 0;
}

#endif