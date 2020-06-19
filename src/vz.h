#ifndef VZ_H_
#define VZ_H_

#include <stdint.h>
#include "vzcontext.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t	vz_magic;
	uint8_t		vz_name[17];
	uint8_t		vz_type;
	uint16_t	vz_start;
	uint8_t		dat[0];
} __attribute__((__packed__)) vz_header;

	
#define	VZ_BASIC		0xF0
#define	VZ_MCODE		0xF1

//VZ_MAGIC 0x20 0x20 0x00 0x00
//VZ_MAGIC 0x56 0x5a 0x46 0x30 VZF

#define	VZ_MAGIC		0x30465A56
#define	VZ_MAGIC_OTHER	0x00002020

uint32_t vz_parse(uint8_t* buf, uint32_t buf_len, uint8_t* vz_name, uint8_t *vz_type, uint16_t *vz_start, uint16_t *vz_len, uint8_t *dat);

int vz_load(VZCONTEXT *context, uint8_t vz_type, uint16_t vz_start, uint16_t vz_len, uint8_t *dat);

#ifdef __cplusplus
}
#endif

#endif	// VZ_H_
