#ifndef DSK_H_
#define DSK_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int dsk_load(uint8_t* buf, uint8_t *dsk_dat, int dsk_len);

#ifdef __cplusplus
}
#endif

#endif	// DSK_H_
