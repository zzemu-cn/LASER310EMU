#ifndef EMU_CORE_H_
#define EMU_CORE_H_

#include "vzcontext.h"
#include "fd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CHARROM_SIZE  0x0C00    //3KB
#define SYSROM_SIZE   0x4000    //16KB
#define DOSROM_SIZE   0x2000    //8KB
#define VRAM_SIZE     0x2000    //8KB
#define RAM_SIZE      0x10000   //64KB

typedef void (*ScreenUpdateCallback)();

extern VZCONTEXT	vzcontext;

#define AUDIO_BUF_MAXLEN (64*64)
extern int16_t audio_buf[AUDIO_BUF_MAXLEN];
extern int audio_buf_pos;
extern int16_t audio_volume;
extern int audio_data_cnt;
extern int audio_play_cnt;

extern char		vz_name[18];
extern uint8_t	vz_type;
extern uint16_t	vz_start;
extern uint16_t	vz_len;

extern unsigned char *screenData;
extern uint8_t	vz_dat[0x10000];

extern unsigned int	systemRunning;

/* emulator control */
extern void emu_setScreenUpdateCallback(ScreenUpdateCallback callback);
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
extern uint8_t emu_sysio_output(void *context, uint8_t port, uint8_t x);

#ifdef __cplusplus
}
#endif

#endif	// EMU_CORE_H_
