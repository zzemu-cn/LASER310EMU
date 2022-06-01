#include <SDL2/SDL.h>

#include "utils/bithacks.h"

#include "plat/plat.h"
#include "plat/win/plat_win.h"

#include "gbldefs.h"
#include "win_gblvar.h"

#include "emu.h"
#include "emu_core.h"

/*
#define DEBUG
#include "dbg.h"

#ifdef DBG_OSTREAM
#undef DBG_OSTREAM
#define DBG_OSTREAM dbg_file
#endif
*/

//----------------------------------------//
// SDL defined type variables, arrays, etc//
//----------------------------------------//
SDL_Color colors[10];
SDL_Event event;
SDL_Window *sdlWindow;
SDL_Surface *sdlScreen;
SDL_Texture *sdlTexture;
SDL_Renderer *sdlRenderer;
SDL_DisplayMode displayMode;
SDL_AudioDeviceID sdlAudioDev;
//static 
SDL_TimerID fpsTimerID = 0;
//----------------------------------------//

void SetupSDLTimers();
void UpdateScreen();

int sdlwin_ready = 0;

//----------------------------------------//
// External data.                         //
//----------------------------------------//

// External functions.
extern int InitializeSound();

// External variables and arrays.

extern unsigned char joyState[9];
extern unsigned char screenData[0xC000];
extern unsigned int CPURunning;
extern unsigned long long int FPS;
extern unsigned int FPSLimit;
extern unsigned int SpeedKey;

extern unsigned char szFileName[];
//----------------------------------------//


//----------------------------------------//
// Constants.                             //
//----------------------------------------//

const unsigned int SCREEN_BPP = 8;
//----------------------------------------//


//----------------------------------------//
// Miscellaneous variables.               //
//----------------------------------------//

boolean fullScreenOn = 0;


//----------------------------------------//
// Miscellaneous pointers.                //
//----------------------------------------//

//----------------------------------------//
// Screen buffer arrays.                  //
//----------------------------------------//
//unsigned char oldScreenData[256 * 192];
//unsigned char screenData[256 * 192];

//----------------------------------------//

int Emu_EventFilter(void *userdata, union SDL_Event *event);


// 50*312*227.5*5 = 17745000
// 44100Hz
// 17745000/5/44100 = 80.476
// 24000Hz
// 17745000/5/24000 = 147.875

SDL_AudioSpec sdl_audio, sdl_audio_have;
int audio_stream_pos;

void audio_callback(void *data, unsigned char *stream, int len)
{
	int i, pos, c;
	pos = audio_buf_pos;
	if(audio_buf_pos<audio_stream_pos)	pos+=AUDIO_BUF_MAXLEN;

	//DBG("%d %d %d", audio_stream_pos, audio_buf_pos, len);

	// 跳播放数据
	if(pos-audio_stream_pos > AUDIO_BUF_MAXLEN/2) {
		audio_stream_pos+=len/sizeof(int16_t);
		audio_stream_pos%=AUDIO_BUF_MAXLEN;
		//DBG("skip");
	}

	char *p;
	if(pos-audio_stream_pos>=len/sizeof(int16_t)) {
		p = (char*)&(audio_buf[audio_stream_pos]);
		for(i=0; i<len; i++) stream[i] = p[i];

		audio_play_cnt+=len/sizeof(int16_t);

		audio_stream_pos+=len/sizeof(int16_t);
		audio_stream_pos%=AUDIO_BUF_MAXLEN;
	} else {
		// 无播放数据
		for(i=0; i<len; i++) stream[i] = audio_volume;
	}
}

//----------------------------------------//
// Initialize the functions that will be  //
// used in the program.                   //
//----------------------------------------//
int InitializeSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == -1)
		return -1;

	audio_volume = 0;
	audio_stream_pos = 0;

	SDL_memset(&sdl_audio, 0, sizeof(sdl_audio));

	sdl_audio.freq = 24000;
	sdl_audio.format = AUDIO_S16; //AUDIO_S16LSB; //AUDIO_S16MSB; //AUDIO_S8;
	sdl_audio.channels = 1;
	sdl_audio.silence = 0;
	sdl_audio.samples = 64*2;
	sdl_audio.size	= 64*sizeof(int16_t);
	sdl_audio.callback = audio_callback;

/*
#ifdef _WIN32
	sdl_audio.samples = 512;
#else
	sdl_audio.samples = 4096;
#endif
*/
	//sdlAudioDev = SDL_OpenAudioDevice(NULL, 0, &sdl_audio, &sdl_audio_have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	sdlAudioDev = SDL_OpenAudioDevice(NULL, 0, &sdl_audio, &sdl_audio_have, 0);
	SDL_PauseAudioDevice(sdlAudioDev, 0);
	//SDL_Delay();
	return 0;
}

//----------------------------------------//
// Change the width and height of the     //
// display.                               //
//----------------------------------------//
void ResizeScreen(const int w, const int h)
{
	SDL_SetWindowSize(sdlWindow, w, h);
}

//----------------------------------------//
// Open the SDL window, set up the palette//
//----------------------------------------//
int OpenSDLWindow(HWND hWnd, const int w, const int h)
{
	sdlwin_ready = 0;	Sleep(10); // 等窗口更新完成

	fullScreenOn = 0;

	sdlWindow = SDL_CreateWindowFrom(hWnd);
	if (sdlWindow == NULL) return 0;

	sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_SOFTWARE);
	//sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
	if (sdlRenderer == NULL) return 0;
	Sleep(10);
	SDL_RenderSetLogicalSize(sdlRenderer, SCREEN_RES_W, SCREEN_RES_H);

	// Set up a pointer to the sdlScreen
	//screen_ptr = (unsigned char *)sdlScreen->pixels;

	ResizeScreen(w, h);

	//memset(&oldScreenData, 0, 0xC000);
	//memset(&screenData, 0, 0xC000);

	//Uint32 rmask, gmask, bmask, amask;
	//rmask = 0xFF000000; gmask = 0x00FF0000; bmask = 0x0000FF00; amask = 0x000000FF;	
	//sdlScreen = SDL_CreateRGBSurfaceFrom(&screenData, SCREEN_RES_W, SCREEN_RES_H, SCREEN_BPP, SCREEN_RES_W*4, rmask, gmask, bmask, amask);
	sdlScreen = SDL_CreateRGBSurfaceFrom(&screenData, SCREEN_RES_W, SCREEN_RES_H, SCREEN_BPP, SCREEN_RES_W*1, 0, 0, 0, 0);

/*
3'b000	24'h07ff00 : // GREEN
3'b001	24'hffff00 : // YELLOW
3'b010	24'h3b08ff : // BLUE
3'b011	24'hcc003b : // RED
3'b100	24'hffffff : // BUFF
3'b101	24'h07e399 : // CYAN
3'b110	24'hff1cff : // MAGENTA
3'b111	24'hff8100 ; // ORANGE

border		24'h000000
background	24'h07ff00
*/

	colors[0].r = 0x07;	colors[0].g = 0xff;	colors[0].b = 0x00; colors[0].a = 0xff;
	colors[1].r = 0xff;	colors[1].g = 0xff;	colors[1].b = 0x00; colors[1].a = 0xff;
	colors[2].r = 0x3b;	colors[2].g = 0x08;	colors[2].b = 0xff; colors[2].a = 0xff;
	colors[3].r = 0xcc;	colors[3].g = 0x00;	colors[3].b = 0x3b; colors[3].a = 0xff;
	colors[4].r = 0xff;	colors[4].g = 0xff;	colors[4].b = 0xff; colors[4].a = 0xff;
	colors[5].r = 0x07;	colors[5].g = 0xe3;	colors[5].b = 0x99; colors[5].a = 0xff;
	colors[6].r = 0xff;	colors[6].g = 0x1c;	colors[6].b = 0xff; colors[6].a = 0xff;
	colors[7].r = 0xff;	colors[7].g = 0x81;	colors[7].b = 0x00; colors[7].a = 0xff;
	
	colors[8].r = 0x00;	colors[8].g = 0x00;	colors[8].b = 0x00; colors[8].a = 0xff;
	colors[9].r = 0x07;	colors[9].g = 0xff;	colors[9].b = 0x00; colors[9].a = 0xff;

	SDL_SetPaletteColors(sdlScreen->format->palette, colors, 0, 10);

	sdlwin_ready = 1;

	return -1;
}

//----------------------------------------//
// Close down all SDL functions.          //
//----------------------------------------//
void CloseSDL()
{
	sdlwin_ready = 0;	Sleep(10); // 等窗口更新完成

	SDL_CloseAudioDevice(sdlAudioDev);

	SDL_Quit();
}

//----------------------------------------//
// This updates the FPS display in the    //
// window's title bar.                    //
//----------------------------------------//
unsigned int UpdateFPS(Uint32 interval, void *param)
{
	char windowTitle[256];
	
	//windowTitle = SDL_GetWindowTitle(window);
	
	if(vz_len>0)
		sprintf(windowTitle, "LASER310 FPS: %d MODE: %02X(M%d P%d) PC: %04X D1: T%d D2: T%d VZ: %s %04X %04X", FPS, vzcontext.latched_shrg, vzcontext.latched_shrg>>2, vzcontext.latched_shrg&0x03, vzcontext.state.pc, fd_track_d1, fd_track_d2, vz_name, vz_start, vz_len);
	else
		sprintf(windowTitle, "LASER310 FPS: %d MODE: %02X(M%d P%d) PC: %04X D1: T%d D2: T%d", FPS, vzcontext.latched_shrg, vzcontext.latched_shrg>>2, vzcontext.latched_shrg&0x03, vzcontext.state.pc, fd_track_d1, fd_track_d2);
	SDL_SetWindowTitle(sdlWindow, windowTitle);

	FPS = 0;
	
	return interval;
}

void AddFPSTimer()
{
	if (!fpsTimerID)
		fpsTimerID = SDL_AddTimer(1000, UpdateFPS, 0);
}

void RemoveFPSTimer()
{
	if (fpsTimerID)
		fpsTimerID = SDL_RemoveTimer(fpsTimerID);
}

//----------------------------------------//
// This will lock the emulation to        //
// roughly 60 frames a second.            //
//----------------------------------------//
void LimitFPS()
{
	static unsigned long int emulationTimer = 0;
	unsigned int timeDifference;
	
	if (!emulationTimer)
		emulationTimer = SDL_GetTicks();

	timeDifference = SDL_GetTicks() - emulationTimer;

	// Display one frame per 1/60 of a second.
	if (timeDifference < 16.6)
		SDL_Delay(16.6 - timeDifference);

	emulationTimer = SDL_GetTicks();
}


//----------------------------------------//
// This gets the current state of the     //
// keyboard and, depending on the keys,   //
// takes the appropriate actions.         //
//----------------------------------------//

//----------------------------------------//
// This function draws the actual sdlScreen  //
// data to the surface then displays it.  //
//----------------------------------------//
void UpdateScreen()
{
	if(sdlwin_ready) {
		SDL_memcpy(sdlScreen->pixels, screenData, 0xC000);
		sdlTexture = SDL_CreateTextureFromSurface(sdlRenderer, sdlScreen);
		SDL_RenderClear(sdlRenderer);
		SDL_RenderCopy(sdlRenderer, sdlTexture, 0, 0);
		SDL_DestroyTexture(sdlTexture);
		SDL_RenderPresent(sdlRenderer);
	}
}


int key_map(const SDL_Keysym *keysym, int*i, int*j)
{
/*
	switch (keysym->sym) {
		case SDLK_r:			*i=0;*j=5;	break;
	}
*/
	*i=-1;*j=-1;
	switch (keysym->scancode) {
		case SDL_SCANCODE_R:		*i=0;*j=5;	break;
		case SDL_SCANCODE_Q:		*i=0;*j=4;	break;
		case SDL_SCANCODE_E:		*i=0;*j=3;	break;
		//case SDL_SCANCODE_:		*i=0;*j=2;	break;
		case SDL_SCANCODE_W:		*i=0;*j=1;	break;
		case SDL_SCANCODE_T:		*i=0;*j=0;	break;

		case SDL_SCANCODE_F:		*i=1;*j=5;	break;
		case SDL_SCANCODE_A:		*i=1;*j=4;	break;
		case SDL_SCANCODE_D:		*i=1;*j=3;	break;
		case SDL_SCANCODE_LCTRL:
		case SDL_SCANCODE_RCTRL:	*i=1;*j=2;	break;
		case SDL_SCANCODE_S:		*i=1;*j=1;	break;
		case SDL_SCANCODE_G:		*i=1;*j=0;	break;

		case SDL_SCANCODE_V:		*i=2;*j=5;	break;
		case SDL_SCANCODE_Z:		*i=2;*j=4;	break;
		case SDL_SCANCODE_C:		*i=2;*j=3;	break;
		case SDL_SCANCODE_LSHIFT:
		case SDL_SCANCODE_RSHIFT:	*i=2;*j=2;	break;
		case SDL_SCANCODE_X:		*i=2;*j=1;	break;
		case SDL_SCANCODE_B:		*i=2;*j=0;	break;

		case SDL_SCANCODE_4:		*i=3;*j=5;	break;
		case SDL_SCANCODE_1:		*i=3;*j=4;	break;
		case SDL_SCANCODE_3:		*i=3;*j=3;	break;
		//case SDL_SCANCODE_:		*i=3;*j=2;	break;
		case SDL_SCANCODE_2:		*i=3;*j=1;	break;
		case SDL_SCANCODE_5:		*i=3;*j=0;	break;

		case SDL_SCANCODE_M:		*i=4;*j=5;	break;
		case SDL_SCANCODE_SPACE:	*i=4;*j=4;	break;
		case SDL_SCANCODE_COMMA:	*i=4;*j=3;	break;
		//case SDL_SCANCODE_:		*i=4;*j=2;	break;
		case SDL_SCANCODE_PERIOD:	*i=4;*j=1;	break;
		case SDL_SCANCODE_N:		*i=4;*j=0;	break;

		case SDL_SCANCODE_7:		*i=5;*j=5;	break;
		case SDL_SCANCODE_0:		*i=5;*j=4;	break;
		case SDL_SCANCODE_8:		*i=5;*j=3;	break;
		case SDL_SCANCODE_MINUS:	*i=5;*j=2;	break;
		case SDL_SCANCODE_9:		*i=5;*j=1;	break;
		case SDL_SCANCODE_6:		*i=5;*j=0;	break;

		case SDL_SCANCODE_U:		*i=6;*j=5;	break;
		case SDL_SCANCODE_P:		*i=6;*j=4;	break;
		case SDL_SCANCODE_I:		*i=6;*j=3;	break;
		case SDL_SCANCODE_RETURN:	*i=6;*j=2;	break;
		case SDL_SCANCODE_O:		*i=6;*j=1;	break;
		case SDL_SCANCODE_Y:		*i=6;*j=0;	break;

		case SDL_SCANCODE_J:		*i=7;*j=5;	break;
		case SDL_SCANCODE_SEMICOLON:*i=7;*j=4;	break;
		case SDL_SCANCODE_K:		*i=7;*j=3;	break;
		case SDL_SCANCODE_APOSTROPHE:	*i=7;*j=2;	break;
		case SDL_SCANCODE_L:		*i=7;*j=1;	break;
		case SDL_SCANCODE_H:		*i=7;*j=0;	break;

		// 扩展按键  scancode[8] 上下左右 Backspace TAB ESC `  scancode[9] = [ ] \ / LALT RALT
		case SDL_SCANCODE_UP:		*i=8;*j=0;	break;
		case SDL_SCANCODE_DOWN:		*i=8;*j=1;	break;
		case SDL_SCANCODE_LEFT:		*i=8;*j=2;	break;
		case SDL_SCANCODE_RIGHT:	*i=8;*j=3;	break;
		case SDL_SCANCODE_BACKSPACE:*i=8;*j=4;	break;
		case SDL_SCANCODE_TAB:		*i=8;*j=5;	break;
		case SDL_SCANCODE_ESCAPE:	*i=8;*j=6;	break;
		case SDL_SCANCODE_NONUSHASH:*i=8;*j=7;	break;

		case SDL_SCANCODE_EQUALS:		*i=9;*j=1;	break;
		case SDL_SCANCODE_LEFTBRACKET:	*i=9;*j=2;	break;
		case SDL_SCANCODE_RIGHTBRACKET:	*i=9;*j=3;	break;
		case SDL_SCANCODE_BACKSLASH:	*i=9;*j=4;	break;
		case SDL_SCANCODE_SLASH:		*i=9;*j=5;	break;
		case SDL_SCANCODE_LALT:			*i=9;*j=6;	break;
		case SDL_SCANCODE_RALT:			*i=9;*j=7;	break;
		//case SDL_SCANCODE_F1:					break;
	}

	return (*i==-1)?0:1;
}

int Emu_EventFilter(void *userdata, union SDL_Event *event)
{
	int i, j;

    switch(event->type) {
		case SDL_KEYDOWN:
			//plat_beep();
			if(key_map(&event->key.keysym, &i,&j)) B_SET(vzcontext.scancode[i],j);
			return 0;
        //break;
		case SDL_KEYUP:
			if(key_map(&event->key.keysym, &i,&j)) B_UNSET(vzcontext.scancode[i],j);

			if(event->key.keysym.sym==SDLK_F11) {
				if(fullScreenOn) {
					sdlwin_ready = 0;	Sleep(10); // 等窗口更新完成

					SDL_SetWindowFullscreen(sdlWindow, 0);
					//SDL_ShowCursor(SDL_ENABLE);
					fullScreenOn = 0;
					Sleep(10);

					sdlwin_ready = 1;
				} else {
					sdlwin_ready = 0;	Sleep(10); // 等窗口更新完成

					SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN);
					//SDL_ShowCursor(SDL_DISABLE);
					fullScreenOn = SDL_WINDOW_FULLSCREEN;
					Sleep(10);

					sdlwin_ready = 1;
				}
			}

			if(event->key.keysym.sym==SDLK_F1) {
				if(vzcontext.vkey_len==0) {
					sdlwin_ready = 0;	Sleep(10); // 等窗口更新完成

					strcpy(vzcontext.vkey,"RUN\n");
					vzcontext.vkey_cur = 0;
					vzcontext.vkey_len = 4;

					sdlwin_ready = 1;
				}
			}
			return 0;
        //break;
		case SDL_MOUSEBUTTONDOWN:
			if(event->button.button==SDL_BUTTON_RIGHT)
				if(vzcontext.vkey_len==0) {
					sdlwin_ready = 0;	Sleep(10); // 等窗口更新完成

					GetTextFromClipboard(vzcontext.vkey);
					vzcontext.vkey_cur = 0;
					vzcontext.vkey_len = strlen(vzcontext.vkey);

					sdlwin_ready = 1;
				}
			return 0;
        //break;
    }
    return 1;
}

