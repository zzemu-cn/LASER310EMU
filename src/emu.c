#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "plat/plat.h"
#include "emu_core.h"

// An array that holds the pixel data that will actually be drawn to the screen.
//unsigned char screenData[0xC000];	// 256 *192
//unsigned char mc8247font[0x1000];

//----------------------------------------//
// This function will initialize the GB   //
// to its startup state.
//----------------------------------------//
int EmulationInitialize(uint8_t *fontrom, uint8_t *sysrom, uint8_t *dosrom)
{
	systemRunning = 0;
	plat_delay_ms(100);

	memcpy(vzcontext.mc8247font, fontrom, 1024*3);
	memset(vzcontext.memory, 0xff, 1024*64);
	memset(vzcontext.vram, 0xff, 1024*8);
	memcpy(vzcontext.memory + 0x0000, sysrom, 1024*16);
	memcpy(vzcontext.memory + 0x4000, dosrom, 1024*8);

	memset(vzcontext.scancode, 0x00, sizeof(vzcontext.scancode)/sizeof(uint8_t));
	memset(vzcontext.vscancode, 0x00, sizeof(vzcontext.vscancode)/sizeof(uint8_t));

	vzcontext.vkey_len=0;
	vzcontext.vkey_cur=0;

	vzcontext.latched_ga = 0xff;
	vzcontext.latched_shrg = 0x08;
	//vzcontext.latched_shrg = 0x00;

	/* Emulate. */
	Z80Reset(&vzcontext.state);
	vzcontext.state.pc = 0x0000;
	
	// vz fd
	fd_track_d1 = 0;
	fd_track_d2 = 0;
	fd_pos = 0;
	fdc_poll_q = 0;
	fdc_poll_dat = 0;
	fd_poll_pos = 0;
	fd_ct_latch = 0;
}


unsigned long long int FPS = 0;
//unsigned int SpeedKey = 0;

void RunEmulation()
{
	systemRunning = 1;
}


void PauseEmulation()
{
	systemRunning = 0;
	plat_delay_ms(100);
}
