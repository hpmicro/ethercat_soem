/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <osal.h>
#include <board.h>
#include "hpm_clock_drv.h"
#include "hpm_csr_drv.h"
#if defined(CONFIG_FREERTOS) && CONFIG_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

#define timercmp(a, b, CMP)               \
    (((a)->tv_sec == (b)->tv_sec) ?       \
         ((a)->tv_usec CMP(b)->tv_usec) : \
         ((a)->tv_sec CMP(b)->tv_sec))
#define timeradd(a, b, result)                           \
    do {                                                 \
        (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;    \
        (result)->tv_usec = (a)->tv_usec + (b)->tv_usec; \
        if ((result)->tv_usec >= 1000000) {              \
            ++(result)->tv_sec;                          \
            (result)->tv_usec -= 1000000;                \
        }                                                \
    } while (0)
#define timersub(a, b, result)                           \
    do {                                                 \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
        (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((result)->tv_usec < 0) {                     \
            --(result)->tv_sec;                          \
            (result)->tv_usec += 1000000;                \
        }                                                \
    } while (0)

#define CFG_TICKS_PER_SECOND 1000000
#define USECS_PER_SEC        1000000
#define USECS_PER_TICK       (USECS_PER_SEC / CFG_TICKS_PER_SECOND)

int64_t board_get_us()
{
    return (hpm_csr_get_core_mcycle() / (clock_get_frequency(clock_cpu0) / 1000000));
}

int osal_gettimeofday(struct timeval *tp)
{
    uint64_t tick = board_get_us();
    uint64_t ticks_left;

    tp->tv_sec = tick / CFG_TICKS_PER_SECOND;

    ticks_left = tick % CFG_TICKS_PER_SECOND;
    tp->tv_usec = ticks_left * USECS_PER_TICK;

    return 0;
}

int osal_usleep(uint32 usec)
{
    board_delay_us(usec);
    return 1;
}

ec_timet osal_current_time(void)
{
    struct timeval current_time;
    ec_timet return_value;

    osal_gettimeofday(&current_time);
    return_value.sec = current_time.tv_sec;
    return_value.usec = current_time.tv_usec;
    return return_value;
}

void osal_timer_start(osal_timert *self, uint32 timeout_usec)
{
    struct timeval start_time;
    struct timeval timeout;
    struct timeval stop_time;

    osal_gettimeofday(&start_time);
    timeout.tv_sec = timeout_usec / USECS_PER_SEC;
    timeout.tv_usec = timeout_usec % USECS_PER_SEC;
    timeradd(&start_time, &timeout, &stop_time);

    self->stop_time.sec = stop_time.tv_sec;
    self->stop_time.usec = stop_time.tv_usec;
}

boolean osal_timer_is_expired(osal_timert *self)
{
    struct timeval current_time;
    struct timeval stop_time;
    int is_not_yet_expired;

    osal_gettimeofday(&current_time);
    stop_time.tv_sec = self->stop_time.sec;
    stop_time.tv_usec = self->stop_time.usec;
    is_not_yet_expired = timercmp(&current_time, &stop_time, <);

    return is_not_yet_expired == false;
}

void *osal_malloc(size_t size)
{
#if defined(CONFIG_FREERTOS) && CONFIG_FREERTOS
    return pvPortMalloc(size);
#else
    return malloc(size);
#endif
}

void osal_free(void *ptr)
{
#if defined(CONFIG_FREERTOS) && CONFIG_FREERTOS
    vPortFree(ptr);
#else
    free(ptr);
#endif
}

int osal_thread_create(void *thandle, int stacksize, void *func, void *param)
{
    void (*thread_func)(void *) = func;
    (void) stacksize;
    (void) thandle;

    thread_func(param);
    return 1;
}