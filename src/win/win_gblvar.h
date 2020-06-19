#ifndef WIN_GBLVAR_H_
#define WIN_GBLVAR_H_

//#define UNICODE

# define BITMAP WINDOWS_BITMAP
# if 0
#  ifdef _WIN32_WINNT
#   undef _WIN32_WINNT
#   define _WIN32_WINNT 0x0501
#  endif
# endif
# include <windows.h>
# include <tchar.h>
# include "resource.h"
# undef BITMAP

/* Class names and such. */
#define CLASS_NAME			_T("AppMainWnd")
#define MENU_NAME			_T("MainMenu")

/*
#define ACCEL_NAME		_T("MainAccel")
#define SUB_CLASS_NAME		_T("AppSubWnd")
#define SB_CLASS_NAME		_T("AppStatusBar")
#define SB_MENU_NAME		_T("StatusBarMenu")
#define FS_CLASS_NAME		_T("AppFullScreen")
*/


extern HINSTANCE	hInstance;
extern HWND			hwndMain;
extern HANDLE		thMain;

#define MAXPATH 1024

#endif	// WIN_GBLVAR_H
