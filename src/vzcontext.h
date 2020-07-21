/* vzcontext.h
 * Header for vzcontext example.
 *
 * Copyright (c) 2012, 2016 Lin Ke-Fong
 *
 * This code is free, do whatever you want with it.
 */

#ifndef VZCONTEXT_H_
#define VZCONTEXT_H_

#include <stdint.h>
#include "z80emu.h"

/*
   KD5 KD4 KD3 KD2 KD1 KD0 扫描用地址
A0  R   Q   E       W   T  68FEH       0
A1  F   A   D  CTRL S   G  68FDH       8
A2  V   Z   C  SHFT X   B  68FBH      16
A3  4   1   3       2   5  68F7H      24
A4  M  空格 ，      .   N  68EFH      32
A5  7   0   8   -   9   6  68DFH      40
A6  U   P   I  RETN O   Y  68BFH      48
A7  J   ；  K   :   L   H  687FH      56

// 键盘选通，整个竖列有一个选通的位置被按下，对应值为0。
*/

// 64 x 64 x 4		gm: 000
// 128 x 64 x 2		gm: 001
// 128 x 64 x 4 	gm: 010
// 128 x 96 x 2		gm: 011
// 128 x 96 x 4		gm: 100
// 128 x 192 x 2	gm: 101
// 128 x 192 x 4	gm: 110
// 256 x 192 x 2	gm: 111

// palette
/*
位\色  绿   黄   蓝   红   浅黄  浅蓝  紫   橙
D6     0    0    0    0    1     1     1    1
D5     0    0    1    1    0     0     1    1
D4     0    1    0    1    0     1     0    1

0x07 0xff 0x00 // GREEN
0xff 0xff 0x00 // YELLOW
0x3b 0x08 0xff // BLUE
0xcc 0x00 0x3b // RED
0xff 0xff 0xff // BUFF
0x07 0xe3 0x99 // CYAN
0xff 0x1c 0xff // MAGENTA
0xff 0x81 0x00 // ORANGE

0x00 0x00 0x00 // BLACK
0x07 0xff 0x00 // GREEN
0x3b 0x08 0xff // BLUE
0xff 0xff 0xff // BUFF

*/

#define	VKEY_MAXLEN	(32*1024)

typedef struct {
	Z80_STATE	state;
	//unsigned char	memory[0x10000];
	uint8_t		memory[0x10000];
	uint8_t		vram[0x2000];
	uint8_t		scancode[10];	// 扩展按键  scancode[8] 上下左右 Backspace TAB ESC `  scancode[9] = [ ] \ / LALT RALT
	uint8_t		vscancode[10];	// 虚拟按键（自动输入）
	uint8_t		vkey[VKEY_MAXLEN];
	int			vkey_len;
	int			vkey_cur;
	//uint8_t		mc6847gm;
	uint8_t		mc8247font[0x1000];
	uint8_t		latched_ga;
	uint8_t		latched_shrg;
} VZCONTEXT;

extern void     SystemCall (VZCONTEXT *vzcontext);

#endif	// VZCONTEXT_H_

