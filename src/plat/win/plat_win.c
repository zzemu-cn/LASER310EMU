/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		Platform main support module for Windows.
 *
 * Version:	@(#)win.c	1.0.59	2019/11/02
 *
 * Authors:	Sarah Walker, <http://pcem-emulator.co.uk/>
 *		Miran Grca, <mgrca8@gmail.com>
 *		Fred N. van Kempen, <decwiz@yahoo.com>
 *
 *		Copyright 2008-2019 Sarah Walker.
 *		Copyright 2016-2019 Miran Grca.
 *		Copyright 2017-2019 Fred N. van Kempen.
 */
#define UNICODE
#include <windows.h>
#include <shlobj.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#define HAVE_STDARG_H

#define GLOBAL
#include "plat.h"
#include "plat_win.h"


/* Platform Public data, specific. */
HINSTANCE	hinstance;		/* application instance */

/* Local data. */

#ifdef ENABLE_WIN_LOG
int win_do_log = ENABLE_WIN_LOG;

static void
win_log(const char *fmt, ...)
{
    va_list ap;

    if (win_do_log) {
	va_start(ap, fmt);
	pclog_ex(fmt, ap);
	va_end(ap);
    }
}
#else
#define win_log(fmt, ...)
#endif


/* Create a console if we don't already have one. */
static void
CreateConsole(int init)
{
    HANDLE h;
    FILE *fp;
    fpos_t p;
    int i;

    if (! init) {
	FreeConsole();
	return;
    }

    /* Are we logging to a file? */
    p = 0;
    (void)fgetpos(stdout, &p);
    if (p != -1) return;

    /* Not logging to file, attach to console. */
    if (! AttachConsole(ATTACH_PARENT_PROCESS)) {
	/* Parent has no console, create one. */
	if (! AllocConsole()) {
		/* Cannot create console, just give up. */
		return;
	}
    }
    fp = NULL;
    if ((h = GetStdHandle(STD_OUTPUT_HANDLE)) != NULL) {
	/* We got the handle, now open a file descriptor. */
	if ((i = _open_osfhandle((intptr_t)h, _O_TEXT)) != -1) {
		/* We got a file descriptor, now allocate a new stream. */
		if ((fp = _fdopen(i, "w")) != NULL) {
			/* Got the stream, re-initialize stdout without it. */
			(void)freopen("CONOUT$", "w", stdout);
			setvbuf(stdout, NULL, _IONBF, 0);
			fflush(stdout);
		}
	}
    }
}

// 从剪切板中取得数据
bool GetTextFromClipboard(char* buf)
{
	buf[0] = 0;

	if(OpenClipboard(NULL)) {
		// 获得剪贴板数据
		HGLOBAL hMem = GetClipboardData(CF_TEXT);
		if(NULL != hMem) {
			char* lpStr = (char*)GlobalLock(hMem); 
			if(NULL != lpStr) {
				//MessageBoxA(0, lpStr, "Clipboard", MB_OK);
				strcpy_s(buf, CLIP_MAXLEN, lpStr);
				GlobalUnlock(hMem);
			}
		}
		CloseClipboard();
		return TRUE;
	}
	return FALSE;
}
 
/* Process the commandline, and create standard argc/argv array. */
int ProcessCommandLine(wchar_t ***argw)
{
    WCHAR *cmdline;
    wchar_t *argbuf;
    wchar_t **args;
    int argc_max;
    int i, q, argc;

    cmdline = GetCommandLine();
    i = wcslen(cmdline) + 1;
    argbuf = (wchar_t *)malloc(sizeof(wchar_t)*i);
    wcscpy(argbuf, cmdline);

    argc = 0;
    argc_max = 64;
    args = (wchar_t **)malloc(sizeof(wchar_t *) * argc_max);
    if (args == NULL) {
	free(argbuf);
	return(0);
    }

    /* parse commandline into argc/argv format */
    i = 0;
    while (argbuf[i]) {
	while (argbuf[i] == L' ')
		  i++;

	if (argbuf[i]) {
		if ((argbuf[i] == L'\'') || (argbuf[i] == L'"')) {
			q = argbuf[i++];
			if (!argbuf[i])
				break;
		} else
			q = 0;

		args[argc++] = &argbuf[i];

		if (argc >= argc_max) {
			argc_max += 64;
			args = realloc(args, sizeof(wchar_t *)*argc_max);
			if (args == NULL) {
				free(argbuf);
				return(0);
			}
		}

		while ((argbuf[i]) && ((q)
			? (argbuf[i]!=q) : (argbuf[i]!=L' '))) i++;

		if (argbuf[i]) {
			argbuf[i] = 0;
			i++;
		}
	}
    }

    args[argc] = NULL;
    *argw = args;

    return(argc);
}


void
plat_get_exe_name(wchar_t *s, int size)
{
    GetModuleFileName(hinstance, s, size);
}


void
plat_tempfile(wchar_t *bufp, wchar_t *prefix, wchar_t *suffix)
{
    SYSTEMTIME SystemTime;
    char temp[1024];

    if (prefix != NULL)
	sprintf(temp, "%ls-", prefix);
      else
	strcpy(temp, "");

    GetSystemTime(&SystemTime);
    sprintf(&temp[strlen(temp)], "%d%02d%02d-%02d%02d%02d-%03d%ls",
        SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
	SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond,
	SystemTime.wMilliseconds,
	suffix);
    mbstowcs(bufp, temp, strlen(temp)+1);
}


int
plat_getcwd(wchar_t *bufp, int max)
{
    (void)_wgetcwd(bufp, max);

    return(0);
}


int
plat_chdir(wchar_t *path)
{
    return(_wchdir(path));
}


FILE *
plat_fopen(wchar_t *path, wchar_t *mode)
{
    return(_wfopen(path, mode));
}


/* Open a file, using Unicode pathname, with 64bit pointers. */
FILE *
plat_fopen64(const wchar_t *path, const wchar_t *mode)
{
    return(_wfopen(path, mode));
}


void
plat_remove(wchar_t *path)
{
    _wremove(path);
}


/* Make sure a path ends with a trailing (back)slash. */
void
plat_path_slash(wchar_t *path)
{
    if ((path[wcslen(path)-1] != L'\\') &&
	(path[wcslen(path)-1] != L'/')) {
	wcscat(path, L"\\");
    }
}


/* Check if the given path is absolute or not. */
int
plat_path_abs(wchar_t *path)
{
    if ((path[1] == L':') || (path[0] == L'\\') || (path[0] == L'/'))
	return(1);

    return(0);
}


/* Return the last element of a pathname. */
wchar_t *
plat_get_basename(const wchar_t *path)
{
    int c = (int)wcslen(path);

    while (c > 0) {
	if (path[c] == L'/' || path[c] == L'\\')
	   return((wchar_t *)&path[c]);
       c--;
    }

    return((wchar_t *)path);
}


/* Return the 'directory' element of a pathname. */
void
plat_get_dirname(wchar_t *dest, const wchar_t *path)
{
    int c = (int)wcslen(path);
    wchar_t *ptr;

    ptr = (wchar_t *)path;

    while (c > 0) {
	if (path[c] == L'/' || path[c] == L'\\') {
		ptr = (wchar_t *)&path[c];
		break;
	}
 	c--;
    }

    /* Copy to destination. */
    while (path < ptr)
	*dest++ = *path++;
    *dest = L'\0';
}

void
plat_get_dirname_a(char *dest, const char *path)
{
    int c = (int)strlen(path);
    char *ptr;

    ptr = (char *)path;

    while (c > 0) {
	if (path[c] == '/' || path[c] == '\\') {
		ptr = (char *)&path[c];
		break;
	}
 	c--;
    }

    /* Copy to destination. */
    while (path < ptr)
	*dest++ = *path++;
    *dest = '\0';
}


wchar_t *
plat_get_filename(wchar_t *s)
{
    int c = wcslen(s) - 1;

    while (c > 0) {
	if (s[c] == L'/' || s[c] == L'\\')
	   return(&s[c+1]);
       c--;
    }

    return(s);
}


wchar_t *
plat_get_extension(wchar_t *s)
{
    int c = wcslen(s) - 1;

    if (c <= 0)
	return(s);

    while (c && s[c] != L'.')
		c--;

    if (!c)
	return(&s[wcslen(s)]);

    return(&s[c+1]);
}


void
plat_append_filename(wchar_t *dest, wchar_t *s1, wchar_t *s2)
{
    wcscat(dest, s1);
    plat_path_slash(dest);
    wcscat(dest, s2);
}


void
plat_put_backslash(wchar_t *s)
{
    int c = wcslen(s) - 1;

    if (s[c] != L'/' && s[c] != L'\\')
	   s[c] = L'/';
}


int
plat_dir_check(wchar_t *path)
{
    DWORD dwAttrib = GetFileAttributes(path);

    return(((dwAttrib != INVALID_FILE_ATTRIBUTES &&
	   (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))) ? 1 : 0);
}

//TODO, Currently, not used. Switch to use POXIS function instead
// int
// plat_dir_create(wchar_t *path)
// {
//     return((int)SHCreateDirectory(hwndMain, path));
// }


uint64_t
plat_timer_read(void)
{
    LARGE_INTEGER li;

    QueryPerformanceCounter(&li);

    return(li.QuadPart);
}


uint32_t
plat_get_ticks(void)
{
    return(GetTickCount());
}


void
plat_delay_ms(uint32_t count)
{
    Sleep(count);
}


//从机器的扬声器中发出蜂鸣声
void
plat_beep(void)
{
	MessageBeep(0);
}
