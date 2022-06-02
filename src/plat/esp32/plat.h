// ESP32 Platform header
#ifndef PLAT_H_
#define PLAT_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GLOBAL
# define GLOBAL extern
#endif

GLOBAL int quited;

extern TickType_t plat_get_ticks(void);
extern void	plat_delay_ms(uint32_t count);

/* Thread/Task supprot */
typedef void thread_t;

extern void	thread_kill(thread_t *threadid);

#ifdef __cplusplus
}
#endif


#endif  //PLAT_H_