#include <stdio.h>
#include <stdlib.h>
#include <sdl2/sdl.h>
#include <windows.h>

#include "bithacks.h"

#include "plat.h"

#include "emu_core.h"
#include "vkey.h"

/*
#define DEBUG

//#include "dbg.h"

#ifdef DBG_OSTREAM
#undef DBG_OSTREAM
#define DBG_OSTREAM dbg_file
#endif
*/

extern void UpdateScreen();
extern unsigned long long int FPS;

#define Z80_CPU_SPEED           4000000   /* In Hz. */
#define CYCLES_PER_STEP         (Z80_CPU_SPEED / 50)
#define MAXIMUM_STRING_LENGTH   100

VZCONTEXT	vzcontext;

char		vz_name[18];

uint8_t		vz_type;
uint16_t	vz_start;
uint16_t	vz_len;
uint8_t		vz_dat[0x10000];


int emu_vkey_keydown();
int emu_vkey_keyup();
void emu_drawscreen();


// An array that holds the pixel data that will actually be drawn to the screen.
unsigned char screenData[0xC000];	// 256 *192

unsigned int systemRunning;

void
emu_thread(void *param)
{
	uint64_t total;			// 总时钟数
	uint32_t total_1ms;		// 执行时钟数
	uint32_t cycles;

	// 50*312*227.5*5 = 17745000
	// 17745000/5/1000 = 3549
	const uint32_t cycles_1ms = 3549; // 每毫秒执行CPU时钟数

	uint32_t ticks_int, ticks_vkey, interval, delay_vkey, frames;	// 中断计时，虚拟按键计时
	uint32_t old_time, new_time;
	int vkey_keydown;

	total = 0;
	total_1ms = 0;

	interval = 0;
	old_time = plat_get_ticks();

	ticks_int = 0;
	ticks_vkey = 0;

	vzcontext.is_done = 1;
	vkey_keydown = 0;

	//FPS = GetTickCount();
	//FPS = plat_get_ticks();
	//FPS = systemRunning;

	while(!quited) {
		if(!systemRunning) {
			//total = 0;
			interval = 0;
			old_time = plat_get_ticks();
			continue;
		}
		//cyclesRan = 0;  // Reset the total cycles ran.

		new_time = plat_get_ticks();
		interval += (new_time - old_time);
		old_time = new_time;
		interval = (interval>100)?100:interval;

		//FPS = GetTickCount();
		while(interval>0) {
			//total = 0;
		    //Z80Reset(&vzcontext.state);
			//vzcontext.state.pc = 0x0000;

			//while(total<cycles_1ms) {
				//total += Z80Emulate(&vzcontext.state, CYCLES_PER_STEP, &vzcontext);
				cycles = Z80Emulate(&vzcontext.state, cycles_1ms-total_1ms, &vzcontext);
				//int_cycles += total;
				total_1ms += cycles;
				total += cycles;

				total_1ms -= cycles_1ms;
			//}

			interval--;
			ticks_int++;
			ticks_vkey++;

			//FPS = ticks;
			//FPS = systemRunning;
			FPS = vzcontext.state.pc;

			// 每秒 7 个字符
			delay_vkey = 70;
			// 回车键间隔稍长些，便于LASER310系统处理输入
			if( (vkey_keydown==0) && vzcontext.vkey_cur>0 && vzcontext.vkey[vzcontext.vkey_cur-1]=='\n')
				delay_vkey = 200;
			if(ticks_vkey>=delay_vkey) {
				ticks_vkey -= delay_vkey;
				if(vkey_keydown)
					vkey_keydown = emu_vkey_keyup();
				else
					vkey_keydown = emu_vkey_keydown();
			}

			// 每秒 50 帧
			if(ticks_int>=20) {
				emu_drawscreen();
				
				//FPS++;
				//LimitFPS();
				UpdateScreen();
				ticks_int -= 20;

				//DBG("interrupt cycles %d", int_cycles);
				cycles = Z80Interrupt(&vzcontext.state, 0, &vzcontext);
				total_1ms += cycles;
				total += cycles;
				//int_cycles = 0;
				//int_cycles += total;
			}
		}

		//while (!context.is_done);

		//HandleSDLEvents();
	}
}


void
emu_close(thread_t *ptr)
{
    int i;

    /* Wait a while so things can shut down. */
    plat_delay_ms(200);

    /* Claim the video blitter. */
    //startblit();

    /* Terminate the main thread. */
    if (ptr != NULL) {
		thread_kill(ptr);

		/* Wait some more. */
		plat_delay_ms(200);
    }
}

uint8_t emu_fetch_byte(void *context, uint16_t adr)
{
	uint8_t x = emu_read_byte(context, adr);
/*
	DBG("fetch %04X %02X", adr, x);
	DBG("status %d  PC %04X  BC %04X  DE %04X  HL %04X  AF %04X\n", vzcontext.state.status, vzcontext.state.pc,
		vzcontext.state.registers.word[Z80_BC], vzcontext.state.registers.word[Z80_DE], vzcontext.state.registers.word[Z80_HL], vzcontext.state.registers.word[Z80_AF]
	);
*/
	return x;
}

const uint8_t uint8_b0 = 1;
const uint8_t uint8_b1 = 2;
const uint8_t uint8_b2 = 4;
const uint8_t uint8_b3 = 8;
const uint8_t uint8_b4 = 16;
const uint8_t uint8_b5 = 32;
const uint8_t uint8_b6 = 64;
const uint8_t uint8_b7 = 128;

const uint16_t uint16_b0 = 1;
const uint16_t uint16_b1 = 2;
const uint16_t uint16_b2 = 4;
const uint16_t uint16_b3 = 8;
const uint16_t uint16_b4 = 16;
const uint16_t uint16_b5 = 32;
const uint16_t uint16_b6 = 64;
const uint16_t uint16_b7 = 128;

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
*/

uint8_t emu_read_byte(void *context, uint16_t adr)
{
	uint8_t x = 0xff;
	uint16_t mask = 0x0001;
	uint8_t ch;

	x = ((VZCONTEXT *)context)->memory[adr&0xffff];

	if((adr&0xf800)==0x6800) {
		//D5~D0用于读键盘信息
		x = 0x00;
		for(int i=0;i<8;i++) {
			if(!(adr&mask)) {
				x |= ((VZCONTEXT *)context)->scancode[i];
				x |= ((VZCONTEXT *)context)->vscancode[i];
			}
			mask <<=1;
		}

		// 扩展按键  scancode[8] 上下左右 Backspace TAB ESC `  scancode[9] = [ ] \ / LALT RALT
		ch = ((VZCONTEXT *)context)->scancode[8];
		// ESC    CTRL + -
		if(B_IS_SET(ch, 6)) {
			if(!(adr&uint16_b1))	B_SET(x,2);	// ctrl
			if(!(adr&uint16_b5))	B_SET(x,2);	// -
		}
		// 退格键 CTRL + M
		if(B_IS_SET(ch, 4)) {
			if(!(adr&uint16_b1))	B_SET(x,2);	// ctrl
			if(!(adr&uint16_b4))	B_SET(x,5);	// m
		}

		x = ~x;
	}

	//if((adr&0xff00)==0xff00) x = 0xff;

	return x;
}

uint16_t emu_read_word(void *context, uint16_t adr)
{
	uint16_t h, l;
	l = emu_read_byte(context, adr);
	h = emu_read_byte(context, adr+1);
	return (h<<8)|l;
}

uint8_t emu_write_byte(void *context, uint16_t adr, uint8_t x)
{
	if((adr&0xf800)==0x6800) {
		((VZCONTEXT *)context)->latched_ga = x;
	}
	if(adr>=0x7000) {
		((VZCONTEXT *)context)->memory[adr&0xffff] = x;
	}

	return x;
}

uint16_t emu_write_word(void *context, uint16_t adr, uint16_t x)
{
	emu_write_byte(context, adr, x&0xff);
	emu_write_byte(context, adr+1, (x>>8)&0xff);

	return x;
}


uint8_t emu_sysio_input(void *context, uint8_t port)
{
	uint8_t x = 0xff;
	uint8_t ch;

	// joystick
	if((port&0xF0)==0x20) {
		// joystick 1
		if(!(port&0x01)) {
			// 键盘模拟游戏杆输入 上下左右 tab `
			ch = ((VZCONTEXT *)context)->scancode[8];
			if(ch&0x01) B_UNSET(x,0);
			if(ch&0x02) B_UNSET(x,1);
			if(ch&0x04) B_UNSET(x,2);
			if(ch&0x08) B_UNSET(x,3);
			if(ch&0x20) B_UNSET(x,4);	// tab
		}
		if(!(port&0x02)) {
			// 键盘模拟游戏杆输入
			ch = ((VZCONTEXT *)context)->scancode[8];
			if(ch&0x80) B_UNSET(x,4);	// `
		}
/*	
		// joystick 2
		if(!(port&0x04)) {
			// 键盘模拟游戏杆输入  上下左右 tab `
			ch = ((VZCONTEXT *)context)->scancode[8];
			if(ch&0x01) B_UNSET(x,0);
			if(ch&0x02) B_UNSET(x,1);
			if(ch&0x04) B_UNSET(x,2);
			if(ch&0x08) B_UNSET(x,3);
			if(ch&0x20) B_UNSET(x,4);	// tab
		}
		if(!(port&0x08)) {
			// 键盘模拟游戏杆输入
			ch = ((VZCONTEXT *)context)->scancode[8];
			if(ch&0x80) B_UNSET(x,4);	// `
		}
*/
	}

	return x;
}


int emu_vkey_keyup()
{
	memset(vzcontext.vscancode, 0x00, sizeof(vzcontext.vscancode)/sizeof(uint8_t));

	if(vzcontext.vkey_len==0) return 0;
	vzcontext.vkey_cur++;
	if(vzcontext.vkey_cur>=vzcontext.vkey_len) {
		 vzcontext.vkey_len=0;
		 vzcontext.vkey_cur=0;
	}

	return 0;
}


int emu_vkey_keydown()
{
	if(vzcontext.vkey_len==0) return 0;

	uint8_t ch = vzcontext.vkey[vzcontext.vkey_cur];
	if(vkey[ch].i==-1) {
		vzcontext.vkey_cur++;
		return 0;
	}

	if(vkey[ch].shift)
		B_SET(vzcontext.vscancode[2],2);
	B_SET(vzcontext.vscancode[vkey[ch].i],vkey[ch].j);

	return 1;
}


void emu_drawscreen()
{
	//int scanlineY;
	int y, x, r, c;
	int i;
	uint8_t ch, font_bits, bits, b;
	uint8_t color, color_css, color_idx_0, color_idx_1;

	//memset(screenData, 0, 0xC000);

	if(vzcontext.latched_ga&0x08) {
		// 图形方式 128*64*2
		if(vzcontext.latched_ga&0x10)
			color_css = 0x04;
		else
			color_css = 0x00;

		for(y=0;y<64;y++) {
			for(x=0;x<32;x++) {
				i=y*3*256+x*8+3*2;
				bits = vzcontext.memory[0x7000 + y*32 + x];
				for(c=0;c<4;c++) {
					color = bits&0x03 | color_css;
/*
					screenData[y*3*256+    x*8+(3-c)*2]		= color;
					screenData[y*3*256+    x*8+(3-c)*2+1]	= color;
					screenData[y*3*256+256+x*8+(3-c)*2]		= color;
					screenData[y*3*256+256+x*8+(3-c)*2+1]	= color;
					screenData[y*3*256+512+x*8+(3-c)*2]		= color;
					screenData[y*3*256+512+x*8+(3-c)*2+1]	= color;
*/
					// 简单的优化
					screenData[i      ]		= color;
					screenData[i    +1]		= color;
					screenData[i+256  ]		= color;
					screenData[i+256+1]		= color;
					screenData[i+512  ]		= color;
					screenData[i+512+1]		= color;

					i-=2;
					bits>>=2;
				}
			}
		}

/*
		// Draw a single scanline.
		for (scanlineY=0; scanlineY<192; scanlineY++) {
			for(x=0;x<32;x++) {
				bits = vzcontext.memory[0x7000 + scanlineY/3*32 + x];
				for(c=0;c<4;c++) {
					screenData[scanlineY*256+x*8+(3-c)*2] = bits&0x03 | color_css;
					screenData[scanlineY*256+x*8+(3-c)*2+1] = bits&0x03 | color_css;
					bits>>=2;

				}
			}
		}
*/
 	} else {
		// 字符方式 32*16
		for(y=0;y<16;y++) {
			for(x=0;x<32;x++) {
				ch = vzcontext.memory[0x7000 + y*32 + x];
				if(ch&0x80) {
					// 图形字符
					color_idx_0 = 8;
					color_idx_1 = (ch&0x70)>>4;
				} else {
					color_idx_1 = 8;
					color_idx_0 = 0;
				}
				i = y*12*256+x*8;
				for(r=0;r<12;r++) {
					font_bits = vzcontext.mc8247font[ch*12+r];
					b = 1;
					for(c=0;c<8;c++) {
						//screenData[(y*12+r)*256+x*8+c] = (font_bits&b)?color_idx_0:color_idx_1;
						// 简单的优化
						screenData[i+c] = (font_bits&b)?color_idx_0:color_idx_1;
						b<<=1;
					}
					i+=256;
				}
			}
		}

/*
		// Draw a single scanline.
		for (scanlineY=0; scanlineY<192; scanlineY++) {
			for(x=0;x<32;x++) {
				b = 1;
				ch = vzcontext.memory[0x7000 + scanlineY/12*32 + x];
				font_bits = vzcontext.mc8247font[ch*12+scanlineY%12];
				if(ch&0x80) {
					// 图形字符
					color_idx_0 = 8;
					color_idx_1 = (ch&0x70)>>4;
				} else {
					color_idx_1 = 8;
					color_idx_0 = 0;
				}
				for(c=0;c<8;c++) {
					screenData[scanlineY*256+x*8+c] = (font_bits&b)?color_idx_0:color_idx_1;
					b<<=1;
				}
			}
		}
*/
	}
}

