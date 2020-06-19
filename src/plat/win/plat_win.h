#ifndef PLAT_WIN_H_
#define PLAT_WIN_H_

#define	CLIP_MAXLEN	(32*1024)

int ProcessCommandLine(wchar_t ***argw);

BOOL GetTextFromClipboard(char* buf);

#endif	// PLAT_WIN_H_
