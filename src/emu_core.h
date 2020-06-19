#ifndef EMU_CORE_H_
#define EMU_CORE_H_

#include "vzcontext.h"

#ifdef __cplusplus
extern "C" {
#endif

extern VZCONTEXT	vzcontext;

extern char		vz_name[18];
extern uint8_t	vz_type;
extern uint16_t	vz_start;
extern uint16_t	vz_len;
extern uint8_t	vz_dat[0x10000];

extern unsigned int	systemRunning;

extern void	emu_thread(void *param);
extern void	emu_start(void);
extern void	emu_close(void *threadid);

extern uint8_t emu_fetch_byte(void *context, uint16_t adr);
extern uint16_t emu_fetch_word(void *context, uint16_t adr);
extern uint8_t emu_read_byte(void *context, uint16_t adr);
extern uint16_t emu_read_word(void *context, uint16_t adr);
extern uint8_t emu_write_byte(void *context, uint16_t adr, uint8_t x);
extern uint16_t emu_write_word(void *context, uint16_t adr, uint16_t x);

extern uint8_t emu_sysio_input(void *contex, uint8_t port);

#ifdef __cplusplus
}
#endif

#endif	// EMU_CORE_H_
