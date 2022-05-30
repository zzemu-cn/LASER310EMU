#define GLOBAL
#include "plat.h"

TickType_t plat_get_ticks(void)
{
    return(xTaskGetTickCount());
}

void plat_delay_ms(uint32_t count)
{
    vTaskDelay(count / portTICK_PERIOD_MS);
}
