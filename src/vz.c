#include <stdint.h>
#include <string.h>

#include "vz.h"


uint32_t vz_parse(uint8_t* buf, uint32_t buf_len, uint8_t* vz_name, uint8_t *vz_type, uint16_t *vz_start, uint16_t *vz_len, uint8_t *dat)
{
	uint32_t	c, fn_len;
	uint16_t	vzf_len;
	vz_header*	h;

	*vz_type = 0;
	*vz_start = 0;
	*vz_len = 0;
	vz_name[0] = 0;
 
	if(buf_len <= sizeof(vz_header))	return 0;

	h = (vz_header*)buf;
	vzf_len = buf_len - sizeof(vz_header);

	//printf("VZF_MAGIC %02X %02X %02X %02X\n", buf[0], buf[1], buf[2], buf[3]);

	//VZF_MAGIC 0x20 0x20 0x00 0x00
	//VZF_MAGIC 0x56 0x5a 0x46 0x30 VZF

	if( !(h->vz_magic == VZ_MAGIC || h->vz_magic == VZ_MAGIC_OTHER) ) return 0;

	*vz_len = vzf_len;


	// VZF_FILENAME
	for(c=0;c<17;c++)
		if(h->vz_name[c]=='\0') break;
	fn_len = c;

	for(c=0;c<fn_len;c++)
		vz_name[c] = h->vz_name[c];
	vz_name[fn_len]=0;


	// VZF_TYPE
	*vz_type = h->vz_type;

	// VZF_STARTADDR
	*vz_start = h->vz_start;

	if((uint32_t)vzf_len + (*vz_start) >= 0x10000)	return 0;

	memcpy(dat, h->dat, vzf_len);

	return vzf_len;
}

int vz_load(VZCONTEXT *context, uint8_t vz_type, uint16_t vz_start, uint16_t vz_len, uint8_t *dat)
{
	uint16_t end;
	end = vz_start + vz_len;

	if((uint32_t)vz_start + vz_len >= 0x10000) return 0;

	switch(vz_type) {
	case VZ_BASIC:
		memcpy(context->memory+vz_start, dat, vz_len);
		context->memory[0x78a4] = vz_start % 256; /* start of basic program */
		context->memory[0x78a5] = vz_start / 256;
		context->memory[0x78f9] = end % 256; /* end of basic program */
		context->memory[0x78fa] = end / 256;
		context->memory[0x78fb] = end % 256; /* start variable table */
		context->memory[0x78fc] = end / 256;
		context->memory[0x78fd] = end % 256; /* start free mem, end variable table */
		context->memory[0x78fe] = end / 256;
		break;

	case VZ_MCODE:
		memcpy(context->memory+vz_start, dat, vz_len);
		context->memory[0x788e] = vz_start % 256; /* usr subroutine address */
		context->memory[0x788f] = vz_start / 256;
		context->state.pc = vz_start;
		break;

	default:
		return 0;
		//image.seterror(IMAGE_ERROR_UNSUPPORTED, "Snapshot format not supported.");
		//image.message("Snapshot format not supported.");
		//return image_init_result::FAIL;
	}

	return 1;
}
