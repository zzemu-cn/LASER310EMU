#ifndef PRGDEF_H_
#define PRGDEF_H_

#ifdef __cplusplus
#include <stdint>
#else
#include <stdint.h>
#endif

//TODO, refactor to use dynamic allocation. This utility buffer is shared by fd.c and FileIO.c
//TODO, very large buffer, change to use heap memory
#define TMP_BUF_LEN	100*1024     //100KB for Disk image 

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t tmp_buf[TMP_BUF_LEN];

#ifdef __cplusplus
}
#endif

#endif	//PRGDEF_H_
