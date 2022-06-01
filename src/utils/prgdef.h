#ifndef PRGDEF_H_
#define PRGDEF_H_

#ifdef __cplusplus
#include <stdint>
#else
#include <stdint.h>
#endif

//TODO, BY - need to refactor 
#ifdef  __MINGW64__
#define TMP_BUF_LEN	0x100000        //1MB buffer
#elif   ESP32
#define TMP_BUF_LEN	0x20000         //128KB buffer
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t tmp_buf[TMP_BUF_LEN];

#ifdef __cplusplus
}
#endif

#endif	//PRGDEF_H_
