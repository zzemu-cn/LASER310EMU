#ifndef EMU_H_
#define EMU_H_


/* Configuration values. */
//#define SERIAL_MAX	2
//#define PARALLEL_MAX	1
#define SCREEN_RES_W	256
#define SCREEN_RES_H	192

#if defined(ENABLE_LOG_BREAKPOINT) || defined(ENABLE_VRAM_DUMP)
# define ENABLE_LOG_COMMANDS	1
#endif

#ifdef __cplusplus
extern "C" {
#endif

int EmulationInitialize(uint8_t *fontrom, uint8_t *sysrom, uint8_t *dosrom);
void RunEmulation();
void PauseEmulation();
void StopEmulation(thread_t *threadid);

#ifdef __cplusplus
}
#endif


#endif	//EMU_H_

