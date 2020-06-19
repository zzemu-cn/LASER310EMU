#include <windows.h>
#include <stdlib.h>
#include <wchar.h>


wchar_t* char2wchar(const char* cchar)
{
	wchar_t *m_wchar;
	int len = MultiByteToWideChar(CP_ACP ,0,cchar ,strlen( cchar), NULL,0);
	m_wchar= malloc(sizeof(wchar_t)*(len+1));
	MultiByteToWideChar(CP_ACP ,0,cchar,strlen( cchar),m_wchar,len);
	m_wchar[len]= '\0' ;
	return m_wchar;
}

char* wchar2char(const wchar_t* wchar)
{
	char * m_char;
	int len= WideCharToMultiByte(CP_ACP ,0,wchar ,wcslen( wchar ), NULL,0, NULL ,NULL);
	m_char= malloc(sizeof(wchar_t)*(len+1));
	WideCharToMultiByte(CP_ACP ,0,wchar ,wcslen( wchar ),m_char,len, NULL ,NULL);
	m_char[len]= '\0';
	return m_char;
}

char* UnicodeToUtf8(const wchar_t* unicode)
{
    int len;
    len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
    char *szUtf8 = (char*)malloc(len + 1);
    memset(szUtf8, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, unicode, -1, szUtf8, len, NULL, NULL);
    return szUtf8;
}
