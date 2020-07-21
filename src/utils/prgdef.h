#ifndef PRGDEF_H_
#define PRGDEF_H_

#ifdef __cplusplus
#include <stdint>
#else
#include <stdint.h>
#endif

#define TMP_BUF_LEN	0x100000

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t tmp_buf[TMP_BUF_LEN];

#ifdef __cplusplus
}
#endif

#endif	//PRGDEF_H_
