//#define UNICODE

#include <memory.h>
#include <sdl2/sdl.h>
#include <stdio.h>

#include <windows.h>
#include <tchar.h>
#include <shlobj.h>

/*
#define DEBUG
#include "dbg.h"

#ifdef DBG_OSTREAM
#undef DBG_OSTREAM
#define DBG_OSTREAM dbg_file
#endif
*/

#include "plat.h"
#include "plat_win.h"
#include "wchar2char.h"



#include "gblvar.h"
#include "win_gblvar.h"


#include "vz.h"
#include "dsk.h"
#include "emu.h"
#include "emu_core.h"

#include "resource.h"

HWND hwndMain;	// application main window

//MSG msg;

HACCEL	haccel;			/* handle to accelerator table */
//HWND	hwndRender;		/* machine render window */

HANDLE	thMain;

//----------------------------------------//
// External data                          //
//----------------------------------------//

//----------------------------------------//
// External functions.                    //
//----------------------------------------//
extern void CloseSDL();
extern int  InitializeSDL();
extern void UpdateScreen();

uint8_t* LoadRomFile(char filename[], unsigned long int *fileSize);
extern void AddFPSTimer();
extern void RemoveFPSTimer();
extern int  OpenSDLWindow(HWND hWnd, const int w, const int h);
extern void ResizeScreen(const int w, const int h);

extern int EmulationInitialize(unsigned char *fontrom, unsigned char *sysrom, unsigned char *dosrom);
extern void RunEmulation();
extern void PauseEmulation();
//----------------------------------------//

/* Emulator start/stop support functions. */
void	do_start(void);
void	do_stop(void);

extern int Emu_EventFilter(void *userdata, union SDL_Event *event);

//----------------------------------------//
// External variables.                    //
//----------------------------------------//

extern SDL_Window window;
//extern unsigned int SCREEN_RES_H;
//extern unsigned int SCREEN_RES_W;
//extern unsigned int screenSizeMultiplier;

extern int sdlwin_ready;
//----------------------------------------//

//----------------------------------------//
// Miscellaneous variables.               //
//----------------------------------------//

uint8_t	fontrom[0x1000];
uint8_t sysrom[0x4000];
uint8_t	dosrom[0x2000];
uint8_t	vzfrom[0x10000];

HANDLE hRomFile;
HANDLE logFile;
TCHAR szFileName[MAX_PATH];
unsigned int CPURunning = 0;
unsigned int FPSLimit = 1;
unsigned int logging = 0;
unsigned int emulationInitialized = 0;


unsigned int screenSizeMultiplier = 3;
extern unsigned int windowHeight;
extern unsigned int windowWidth;

size_t SaveFile(const char fn[], char* buf, const size_t sz);

//----------------------------------------//

// Display an error message if there was an invalid opcode ran.
void OpcodeError( LPCTSTR errorText)
{
	MessageBoxA(hwndMain, errorText, _T("err"), MB_OK);
}

//----------------------------------------//
// Windows Callback routine.              //
//----------------------------------------//
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	uint8_t *tmpFileBuffer = 0;
	unsigned long int fileSize = 0;

	switch(msg)	{
	case WM_CREATE:
		{
			if (InitializeSDL() == -1)
			{
				MessageBoxA(hWnd, _T("Could not initialize SDL!"), _T("err"), MB_OK);
				DestroyWindow(hWnd);
				exit(0);
			}
			RunEmulation();
			AddFPSTimer();
		}
	break;
	case WM_CLOSE:
		{
			RemoveFPSTimer();
			CloseSDL();
			DestroyWindow(hWnd);
			//exit(0);
		}
	break;
	case WM_DESTROY:
		{
			CloseSDL();
			PostQuitMessage(0);
		}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_FILE_OPEN_VZ:
			{
				OPENFILENAME ofn;

				ZeroMemory(&ofn, sizeof(ofn));

				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = _T("VZ Files\0*.vz\0");
				ofn.lpstrFile = szFileName;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
				ofn.lpstrDefExt = (LPCTSTR) _T("");

				if(GetOpenFileName(&ofn)) {
					tmpFileBuffer = LoadRomFile(szFileName, &fileSize);
					if (!tmpFileBuffer) {
						MessageBox(hWnd, _T("Error opening VZ file!"), _T("err"), MB_OK);
					} else {
						if(vz_parse(tmpFileBuffer, fileSize, vz_name, &vz_type, &vz_start, &vz_len, vz_dat)) {
							PauseEmulation();

							// 0x1A33 0x040C
							while(vzcontext.state.pc!=0x040C)
								z80emulate(&vzcontext.state, 0, &vzcontext);

							vz_load(&vzcontext, vz_type, vz_start, vz_len, vz_dat);

							RunEmulation();
						}
					}
				}
			}
		break;
		case IDM_FILE_OPEN_DSK1:
		case IDM_FILE_OPEN_DSK2:
			{
				OPENFILENAME ofn;

				ZeroMemory(&ofn, sizeof(ofn));

				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = _T("DSK Files\0*.dsk\0");
				ofn.lpstrFile = szFileName;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
				ofn.lpstrDefExt = (LPCTSTR) _T("");

				if(GetOpenFileName(&ofn)) {
					tmpFileBuffer = LoadRomFile(szFileName, &fileSize);
					if (!tmpFileBuffer) {
						MessageBox(hWnd, _T("Error opening DSK file!"), _T("err"), MB_OK);
					} else {
						int r;
						PauseEmulation();
						if(LOWORD(wParam)==IDM_FILE_OPEN_DSK1)
							r = dsk_load(fd_buf_d1, tmpFileBuffer, fileSize);
						else
							r = dsk_load(fd_buf_d2, tmpFileBuffer, fileSize);
						RunEmulation();
						if(!r)
							MessageBoxA(hWnd, _T("bad formt"), _T("err"), MB_OK);
					}
				}
			}
		break;
		case IDM_FILE_EXIT:
			{
				CloseSDL();
				DestroyWindow(hWnd);
				exit(0);
			}
		break;
		case IDM_EMULATE_RUN:
			{
				RunEmulation();
			}
		break;
		case IDM_EMULATE_PAUSE:
			{
				PauseEmulation();
			}
		break;
		case IDM_EMULATE_RESET:
			{
				EmulationInitialize(fontrom, sysrom, dosrom);
				RunEmulation();
			}
		break;
/*
		case IDM_OPTIONS_USEFPSLIMIT:
			{
			if (FPSLimit == 1)
			{
				RemoveFPSTimer();
				FPSLimit = 0;
			}
			else
			{
				AddFPSTimer();
				FPSLimit = 1;
			}
			}
		break;
*/
		case IDM_OPTIONS_DISPLAYSIZE_1X:
			{
				sdlwin_ready = 0;	Sleep(10); // 等窗口更新完成

				ResizeScreen(SCREEN_RES_W, SCREEN_RES_H);

				sdlwin_ready = 1;
			}
		break;
		case IDM_OPTIONS_DISPLAYSIZE_2X:
			{
				sdlwin_ready = 0;	Sleep(10); // 等窗口更新完成

				ResizeScreen(SCREEN_RES_W*2, SCREEN_RES_H*2);

				sdlwin_ready = 1;
			}
		break;
		case IDM_OPTIONS_DISPLAYSIZE_3X:
			{
				sdlwin_ready = 0;	Sleep(10); // 等窗口更新完成

				ResizeScreen(SCREEN_RES_W*3, SCREEN_RES_H*3);

				sdlwin_ready = 1;
			}
		break;
		case IDM_OPTIONS_DISPLAYSIZE_FULLSCREEN:
			{
				sdlwin_ready = 0;	Sleep(10); // 等窗口更新完成

				//SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN);
				//SDL_SetWindowFullscreen(sdlWindow, 0);

				sdlwin_ready = 1;
			}
		break;

/*
		case IDM_DEBUG_STARTDEBUGGER:
			{
				ShowDebugger();
			}
		break;
*/
		}
	break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	break;
	}
	return 0;
}

//----------------------------------------//
// The main function.  This setups the    //
// main window and message checking.      //
//----------------------------------------//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	TCHAR title[200];
	WNDCLASSEX wc;			/* buffer for main window's class */

	uint8_t *tmpFileBuffer = 0;
	unsigned long int fileSize = 0;

	TCHAR szPath[MAXPATH];
	TCHAR szDirname[MAXPATH];
	TCHAR szFilename[MAXPATH];
	TCHAR szBasename[MAXPATH];
	TCHAR szExtname[MAXPATH];

#ifdef UNICODE
	//MessageBoxW(NULL, L"UNICODE", L"info", MB_OK);
#else
	//MessageBoxA(NULL, "ANSI", "dbg", MB_OK);
#endif

	if( !GetModuleFileName( NULL, szPath, MAXPATH ) ) {
		MessageBoxA(NULL, _T("GetModuleFileName failed"), _T("info"), MB_OK);
		return 0;
	}

	//MessageBox(NULL, szPath, "dbg", MB_OK);

	plat_get_dirname_a(szDirname, szPath);

	//ParseFilename(szPath, szDirname, szFilename, szBasename, szExtname);
	//printf("%s\n%s\n%s\n%s\n%s\n", szPath, szDirname, szFilename, szBasename, szExtname);
	//DBG("path: %s\n", szPath);

	char* path;

	path = szDirname;

	//MessageBox(NULL, path, "dbg", MB_OK);

	char s_fn[MAXPATH];

	sprintf(s_fn, "%s%s", path, "/rom/character set (1983)(vtech).rom");
	tmpFileBuffer = LoadRomFile(s_fn, &fileSize);
	memcpy(fontrom, tmpFileBuffer, 1024*3);

	sprintf(s_fn, "%s%s", path, "/rom/basic v2.0 (1983)(vtech).rom");
	tmpFileBuffer = LoadRomFile(s_fn, &fileSize);
	memcpy(sysrom, tmpFileBuffer, 1024*16);

	//sprintf(s_fn, "%s%s", path, "/rom/dos basic v1.2 (198x)(vtech).rom");
	sprintf(s_fn, "%s%s", path, "/rom/dos basic v1.2 (198x)(vtech)_patch.rom");
	tmpFileBuffer = LoadRomFile(s_fn, &fileSize);
	memcpy(dosrom, tmpFileBuffer, 1024*8);

	EmulationInitialize(fontrom, sysrom, dosrom);

	wchar_t **argw = NULL;
	int	argc;

	/* Process the command line for options. */
	argc = ProcessCommandLine(&argw);

	if(argc>1) {
		wchar_t *wcfn;
		wcfn = argw[1];

		char* fn = wchar2char(wcfn);

		tmpFileBuffer = LoadRomFile(fn, &fileSize);

		//MessageBox(NULL, fn, "dbg", MB_OK);

		free(fn);

		vz_type=0;
		if (!tmpFileBuffer) {
			MessageBox(NULL, _T("Error opening VZ file!"), _T("err"), MB_OK);
		} else {
			vz_parse(tmpFileBuffer, fileSize, vz_name, &vz_type, &vz_start, &vz_len, vz_dat);
		}
	}

	// 自动执行 vz 文件
	if(vz_type) {
		//vzcontext.state.bp = 0x1A33;
		//vzcontext.state.bp = 0x040C;
		//Z80Emulate(&vzcontext.state, 3549*1000, &vzcontext);
		while(vzcontext.state.pc!=0x040C)
			z80emulate(&vzcontext.state, 0, &vzcontext);
		//vzcontext.state.bp = -1;

		//char s[100];
		//sprintf(s, "%04X\n", vzcontext.state.pc);
		//MessageBox(NULL, s, "dbg", MB_OK);

		vz_load(&vzcontext, vz_type, vz_start, vz_len, vz_dat);
	}

	/* Set the application version ID string. */
	sprintf(app_version, "%s v%s", EMU_NAME, EMU_VERSION);

	systemRunning = 0;
	vz_name[0] = 0;
	vz_len = 0;

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = 0;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
	wc.lpszClassName = (LPCTSTR)CLASS_NAME;
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wc))
	{
		MessageBoxA(NULL, _T("Window Registration Failed!"), _T("err"),
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hwndMain = CreateWindowEx(
		WS_EX_WINDOWEDGE, // WS_EX_CLIENTEDGE
		CLASS_NAME,
		_T("LASER310"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_RES_W*3, SCREEN_RES_H*3,
		NULL, NULL, hInstance, NULL);

	if (!OpenSDLWindow(hwndMain, SCREEN_RES_W*3, SCREEN_RES_H*3))
		MessageBox(NULL, _T("SDL Creation Failed!"), _T("err"),
			MB_ICONEXCLAMATION | MB_OK);

	if (hwndMain == NULL) {
		MessageBox(NULL,_T("Window Creation Failed!"), _T("err"),
			MB_ICONEXCLAMATION | MB_OK);
	} else {
		ShowWindow(hwndMain, nCmdShow);
		UpdateWindow(hwndMain);
	}


	MSG msg;			// received-messages buffer
	int bRet;

	do_start();

/*
	// SDL_SetEventFilter(Emu_EventFilter, NULL);
	// 仅仅简单的事件循环不会出错
	while(GetMessage(&msg, hwndMain, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
*/
	SDL_SetEventFilter(Emu_EventFilter, NULL);

	AddFPSTimer();

	/* Run the message loop. It will run until GetMessage() returns 0 */
	while (!quited) {
		bRet = GetMessage(&msg, NULL, 0, 0);
		if ((bRet == 0) || quited) break;

		if (bRet == -1) {
			//fatal("bRet is -1\n");
		}

		if (msg.message == WM_QUIT) {
			quited = 1;
			break;
		}


		if (! TranslateAccelerator(hwndMain, haccel, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	timeEndPeriod(1);

//    if (mouse_capture)
//		plat_mouse_capture(0);

	/* Close down the emulator. */
	do_stop();

	UnregisterClass(CLASS_NAME, hInstance);

	//win_mouse_close();

	// dump fdc buf
	//SaveFile("fd1.bin", fd_buf_d1, FD_TRACK_LEN);

	return 0;

}

/*
 * We do this here since there is platform-specific stuff
 * going on here, and we do it in a function separate from
 * main() so we can call it from the UI module as well.
 */
void
do_start(void)
{
	LARGE_INTEGER qpc;

	/* We have not stopped yet. */
	quited = 0;

	/* Initialize the high-precision timer. */
	timeBeginPeriod(1);
	QueryPerformanceFrequency(&qpc);
	timer_freq = qpc.QuadPart;
	//win_log("Main timer precision: %llu\n", timer_freq);

	/* Start the emulator, really. */
	emu_setScreenUpdateCallback(UpdateScreen);
	thMain = thread_create(emu_thread, &quited);
	SetThreadPriority(thMain, THREAD_PRIORITY_HIGHEST);
}


/* Cleanly stop the emulator. */
void
do_stop(void)
{
	quited = 1;

	plat_delay_ms(100);

	//if (source_hwnd)
	//PostMessage((HWND) (uintptr_t) source_hwnd, WM_HAS_SHUTDOWN, (WPARAM) 0, (LPARAM) hwndMain);

	emu_close(thMain);

	thMain = NULL;
}

