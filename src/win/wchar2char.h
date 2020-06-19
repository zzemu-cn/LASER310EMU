#ifndef WCHAR2CHAR_H_
#define WCHAR2CHAR_H_

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

wchar_t* char2wchar(const char* cchar);
char* wchar2char(const wchar_t* wchar);

char* UnicodeToUtf8(const wchar_t* unicode);

#ifdef __cplusplus
extern "C" {
#endif


#endif	// EMU_CORE_H_
