#include <stdio.h>
#include <stdlib.h>
#include <sdl2/sdl.h>
#include <windows.h>

#include "bithacks.h"

#include "plat.h"

#include "emu_core.h"
#include "vkey.h"
#include "fd.h"

/*
#define DEBUG

#include "dbg.h"

#ifdef DBG_OSTREAM
#undef DBG_OSTREAM
#define DBG_OSTREAM dbg_file
#endif
*/

#define INFO

extern void UpdateScreen();
extern unsigned long long int FPS;

VZCONTEXT	vzcontext;

char		vz_name[18];

uint8_t		vz_type;
uint16_t	vz_start;
uint16_t	vz_len;
uint8_t		vz_dat[0x10000];

// 17745000/5/50 = 3549
// 17745000/5/148 = 23980 每秒声音数据量

// 50*312*227.5*5 = 17745000
// 44100Hz
// 17745000/5/44100 = 80.476
// 24000Hz
// 17745000/5/24000 = 147.875

int16_t audio_buf[AUDIO_BUF_MAXLEN];
int audio_buf_pos;

int16_t audio_volume;
int16_t audio_vol;

int audio_data_cnt;
int audio_play_cnt;

void audio_timeout();

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

	uint32_t audio_time_cnt;

	// 50*312*227.5*5 = 17745000
	// 17745000/5/1000 = 3549
	const uint32_t cycles_1ms = 3549; // 每毫秒执行CPU时钟数

	uint32_t ticks_int, ticks_vkey, interval, delay_vkey, frames;	// 中断计时，虚拟按键计时
	uint32_t old_time, new_time;
	uint32_t one_s_cnt;
	int vkey_keydown;

	uint16_t last_pc;


	total = 0;
	total_1ms = 0;
	audio_time_cnt = 0;

	interval = 0;
	old_time = plat_get_ticks();

	one_s_cnt = 0;
	audio_data_cnt = 0;
	audio_play_cnt = 0;
	audio_volume = 16384;

	ticks_int = 0;
	ticks_vkey = 0;

	// 声音计数
	audio_buf_pos = 0;

	// 软驱位置计数
	fd_pos = 0;

	vkey_keydown = 0;

	FPS=0;

	while(!quited) {
		if(!systemRunning) {
			//total = 0;
			interval = 0;
			old_time = plat_get_ticks();
			continue;
		}

		new_time = plat_get_ticks();
		interval += (new_time - old_time);
		old_time = new_time;

		one_s_cnt += interval;
		if(one_s_cnt>=1000) {
			one_s_cnt-=1000;

			//DBG("%d %d %d", one_s_cnt, audio_data_cnt, audio_play_cnt);

			audio_data_cnt = 0;
			audio_play_cnt = 0;
		}

		interval = (interval>100)?100:interval;

		while(interval>0) {
			//cycles = Z80Emulate(&vzcontext.state, cycles_1ms-total_1ms, &vzcontext);
			cycles = z80emulate(&vzcontext.state, 0, &vzcontext);
/*
			// DOS 执行IDAM难以同步
			if(last_pc==0x5677&&vzcontext.state.pc==0x5673)
				vzcontext.state.pc=0x5675;
			if(last_pc==0x5687&&vzcontext.state.pc==0x5683)
				vzcontext.state.pc=0x5685;
			last_pc = vzcontext.state.pc;

			//if(vzcontext.state.pc==0x5671) fd_pos-=5;
			//if(vzcontext.state.pc==0x5663) fd_pos-=5;
*/
			//INFO("emu %04X %d", vzcontext.state.pc, cycles);
			total += cycles;
			total_1ms += cycles;
			audio_time_cnt += cycles;

			fd_pos += cycles;
			if(fd_pos>=FD_TRACK_LEN)	fd_pos -= FD_TRACK_LEN;

			// 按毫秒计数
			if(total_1ms>=cycles_1ms) {
				total_1ms -= cycles_1ms;

				interval--;
				ticks_int++;
				ticks_vkey++;

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
			}

			// 每秒 50 帧
			if(ticks_int>=20) {
				emu_drawscreen();
				FPS++;
				//LimitFPS();
				UpdateScreen();
				ticks_int -= 20;
				// Z80_INTERRUPT_MODE_1 : RTS 38H
				cycles = Z80Interrupt(&vzcontext.state, 0, &vzcontext);
				total += cycles;
				total_1ms += cycles;
				audio_time_cnt += cycles;
			}

			// 生成声音缓冲
			// 声音采样计数
			//#define SND_TIMEOUT_CYCLES		148
			#define SND_TIMEOUT_CYCLES		147

			if(audio_time_cnt>=SND_TIMEOUT_CYCLES) {
				audio_time_cnt -= SND_TIMEOUT_CYCLES;
				audio_timeout();
			}
		}

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

	if((adr&0xf800)==0x7000) {
		x = ((VZCONTEXT *)context)->vram[(adr&0x07ff)+0x0800*(vzcontext.latched_shrg&0x03)];
	}

	//if((adr&0xff00)==0xff00) x = 0xff;
	//INFO("BR %04X %02X", adr, x);

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
	uint8_t last_x, cur_x;
	if((adr&0xf800)==0x6800) {
		// D5，D0 扬声器 00 11 都是无声 10 01 不同则发声
		last_x = ((VZCONTEXT *)context)->latched_ga&0x21;
		cur_x = x&0x21;
		audio_vol =	(cur_x==0x20)	?	audio_volume:
					(cur_x==0x01)	?	-audio_volume:
										0;
		/*
		if((cur_x==0x20||cur_x==0x01) && last_x!=cur_x) {
			//发声音
			//计算时间
			//total + cycles
			//生成声音缓冲区
		}
		*/
		((VZCONTEXT *)context)->latched_ga = x;
	}
	if((adr&0xf800)==0x7000) {
		((VZCONTEXT *)context)->vram[(adr&0x07ff)+0x0800*(vzcontext.latched_shrg&0x03)] = x;
	}
	if(adr>=0x7800) {
		((VZCONTEXT *)context)->memory[adr&0xffff] = x;
	}

	//INFO("BW %04X %02X", adr, x);

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

	//if(port==ADDRESS_IO_FDC_CT||port==ADDRESS_IO_FDC_DATA||port==ADDRESS_IO_FDC_POLL||port==ADDRESS_IO_FDC_WP)
	if(port==ADDRESS_IO_FDC_WP)
		x = 0;
		//x = 0x80;

	if(port==ADDRESS_IO_FDC_POLL) {
		x = fdc_io_poll();
		//int c = fdc_io_cycles();
		//uint16_t pc = ((VZCONTEXT *)context)->state.pc;
		//if(pc==0x5663 || pc==0x5675 || pc==0x5685)
		//	INFO("adr %04X: FDC_POLL %02X, %02X (%d : %s %02x %02x %d)",((VZCONTEXT *)context)->state.pc,port,x, fd_pos, fd_date_d1(), fd_poll_dat_d1, fdc_dat_latch, c);
	}

	// IDAM 0xFE, 0xE7, 0x18, 0xC3
	if(port==ADDRESS_IO_FDC_DATA) {
		x = fdc_io_data();
		//int c = fdc_io_cycles();
		//uint16_t pc = ((VZCONTEXT *)context)->state.pc;
		//if(pc==0x5542&&x==0xFE || pc==0x55C7&&x==0xE7 || pc==0x564C&&x==0x18 || pc==0x5673 || pc==0x5683 || pc==0x56D1 || pc==0x5746)
		//if(pc==0x5673 || pc==0x5683 || pc==0x56D1 || pc==0x5746)
		//	INFO("adr %04X: FDC_DATA %02X, %02X (%d : %s %02x %02x %d)",((VZCONTEXT *)context)->state.pc,port,x, fd_pos, fd_date_d1(), fd_poll_dat_d1, fdc_dat_latch, c);

		//if(pc==0x57E6 || pc==0x5873) {	// IDAM
		//	fd_idam_sec = ((VZCONTEXT *)context)->state.registers.byte[Z80_D];
		//	INFO("adr %04X: FDC_DATA %02X, %02X (%d : S%02X)",((VZCONTEXT *)context)->state.pc,port,x, fd_pos, fd_idam_sec);
		//}

/*
		// 引入读盘偏差
		if(pc==0x57E6) {
			fd_pos += rand()%5;
			if(fd_pos>=FD_TRACK_LEN) fd_pos -= FD_TRACK_LEN;
		}
*/
	}

	return x;
}

uint8_t emu_sysio_output(void *context, uint8_t port, uint8_t x)
{
	//DBG("out %d, %02x", port, x);
	if(port==32||port==222)
		vzcontext.latched_shrg = x;

	if(port==ADDRESS_IO_FDC_CT)
		fdc_io_ct(x);

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
	int adr;
	uint8_t *p;
	uint8_t ch, font_bits, bits, b;
	uint8_t color, color_idx_0, color_idx_1;
	uint8_t mc6847_gm, mc6847_css;

	// 256 x 192 x 2	gm: 111
	// 128 x 192 x 4	gm: 110
	// 128 x 192 x 2	gm: 101
	// 128 x 96 x 4		gm: 100
	// 128 x 96 x 2		gm: 011
	// 128 x 64 x 4 	gm: 010
	// 128 x 64 x 2		gm: 001
	// 64 x 64 x 4		gm: 000

	//memset(screenData, 0, 0xC000);
	if(vzcontext.latched_ga&0x08) {
		if(vzcontext.latched_ga&0x10)
			mc6847_css = 0x04;
		else
			mc6847_css = 0x00;

		mc6847_gm = (vzcontext.latched_shrg>>2)&0x07;

		switch(mc6847_gm) {
		case 0x07:
			{
				// 图形方式 256*192*2
				for(y=0;y<192;y++) {
					for(x=0;x<32;x++) {
						p = screenData+y*256+x*8;
						bits = vzcontext.vram[y*32 + x];
						for(c=0;c<8;c++) {
							color = (bits>>7)&0x01 | 0x02 | mc6847_css;
							*p	= color;
							p++;
							bits<<=1;
						}
					}
				}
			}
		break;
		case 0x06:
			{
				// 图形方式 128*192*4
				for(y=0;y<192;y++) {
					for(x=0;x<32;x++) {
						p = screenData+y*256+x*4*2;
						bits = vzcontext.vram[y*32 + x];
						for(c=0;c<4;c++) {
							color = (bits>>6)&0x03 | mc6847_css;
							*p	= color;
							p++;
							*p	= color;
							p++;
							bits<<=2;
						}
					}
				}
			}
		break;
		case 0x05:
			{
				// 图形方式 128*192*2
				for(y=0;y<192;y++) {
					for(x=0;x<16;x++) {
						p = screenData+y*256+x*8*2;
						bits = vzcontext.vram[y*16 + x];
						for(c=0;c<8;c++) {
							color = (bits>>7)&0x01 | 0x02 | mc6847_css;
							*p	= color;
							p++;
							*p	= color;
							p++;
							bits<<=1;
						}
					}
				}
			}
		break;
		case 0x04:
			{
				// 图形方式 128*96*4
				for(y=0;y<96;y++) {
					for(x=0;x<32;x++) {
						p = screenData+y*2*256+x*4*2;
						bits = vzcontext.vram[y*32 + x];
						for(c=0;c<4;c++) {
							color = (bits>>6)&0x03 | mc6847_css;
							*p	= color;
							p[256]	= color;
							p++;
							*p	= color;
							p[256]	= color;
							p++;
							bits<<=2;
						}
					}
				}
			}
		break;
		case 0x03:
			{
				// 图形方式 128*96*2
				for(y=0;y<96;y++) {
					for(x=0;x<16;x++) {
						p = screenData+y*2*256+x*8*2;
						bits = vzcontext.vram[y*16 + x];
						for(c=0;c<8;c++) {
							color = (bits>>7)&0x01 | 0x02 | mc6847_css;
							*p	= color;
							p[256]	= color;
							p++;
							*p	= color;
							p[256]	= color;
							p++;
							bits<<=1;
						}
					}
				}
			}
		break;
		case 0x02:
			{
				// 图形方式 128*64*4
				for(y=0;y<64;y++) {
					for(x=0;x<32;x++) {
						p = screenData+y*3*256+x*4*2;
						bits = vzcontext.vram[y*32 + x];
						for(c=0;c<4;c++) {
							color = (bits>>6)&0x03 | mc6847_css;
							p[    0]	= color;
							p[    1]	= color;
							p[256  ]	= color;
							p[256+1]	= color;
							p[512  ]	= color;
							p[512+1]	= color;
							p+=2;
							bits<<=2;
						}
					}
				}
			}
		break;
		case 0x01:
			{
				// 图形方式 128*64*2
				for(y=0;y<64;y++) {
					for(x=0;x<16;x++) {
						p = screenData+y*3*256+x*8*2;
						bits = vzcontext.vram[y*16 + x];
						for(c=0;c<8;c++) {
							color = (bits>>7)&0x01 | 0x02 | mc6847_css;
							p[    0]	= color;
							p[    1]	= color;
							p[256  ]	= color;
							p[256+1]	= color;
							p[512  ]	= color;
							p[512+1]	= color;
							p+=2;
							bits<<=1;
						}
					}
				}
			}
		break;
		default:
			{
				// 图形方式 64*64*4
				for(y=0;y<64;y++) {
					for(x=0;x<16;x++) {
						p = screenData+y*3*256+x*4*4;
						bits = vzcontext.vram[y*16 + x];
						for(c=0;c<4;c++) {
							color = ((bits>>6)&0x03) | mc6847_css;
							p[    0]	= color;
							p[    1]	= color;
							p[    2]	= color;
							p[    3]	= color;
							p[256  ]	= color;
							p[256+1]	= color;
							p[256+2]	= color;
							p[256+3]	= color;
							p[512  ]	= color;
							p[512+1]	= color;
							p[512+2]	= color;
							p[512+3]	= color;
							p+=4;
							bits<<=2;
						}
					}
				}
			}
		}
 	} else {
		// 字符方式 32*16
		for(y=0;y<16;y++) {
			for(x=0;x<32;x++) {
				ch = vzcontext.vram[y*32 + x];
				if(ch&0x80) {
					// 图形字符
					color_idx_0 = 8;
					color_idx_1 = (ch&0x70)>>4;
				} else {
					color_idx_1 = 8;
					color_idx_0 = 0;
				}
				p = screenData+y*12*256+x*8;
				for(r=0;r<12;r++) {
					font_bits = vzcontext.mc8247font[ch*12+r];
					b = 1;
					for(c=0;c<8;c++) {
						//screenData[(y*12+r)*256+x*8+c] = (font_bits&b)?color_idx_0:color_idx_1;
						// 简单的优化
						*p = (font_bits&b)?color_idx_0:color_idx_1;
						p++;
						b<<=1;
					}
					// 下一行
					p+=256-8;
				}
			}
		}
	}
}

void audio_timeout()
{
	// audio_vol
	audio_buf[audio_buf_pos] = audio_vol;
	audio_buf_pos++;
	audio_buf_pos%=AUDIO_BUF_MAXLEN;

	audio_data_cnt++;
}
