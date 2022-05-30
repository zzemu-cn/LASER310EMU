#include "plat.h"

/* Thread/Task support */
void thread_kill(thread_t *threadid)
{
    if (threadid == NULL)
        return;

    vTaskDelete(threadid);
}
