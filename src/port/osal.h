/******************************************************************************
 *                *          ***                    ***
 *              ***          ***                    ***
 * ***  ****  **********     ***        *****       ***  ****          *****
 * *********  **********     ***      *********     ************     *********
 * ****         ***          ***              ***   ***       ****   ***
 * ***          ***  ******  ***      ***********   ***        ****   *****
 * ***          ***  ******  ***    *************   ***        ****      *****
 * ***          ****         ****   ***       ***   ***       ****          ***
 * ***           *******      ***** **************  *************    *********
 * ***             *****        ***   *******   **  **  ******         *****
 *                           t h e  r e a l t i m e  t a r g e t  e x p e r t s
 *
 * http://www.rt-labs.com
 * Copyright (C) 2009. rt-labs AB, Sweden. All rights reserved.
 *------------------------------------------------------------------------------
 * $Id: osal.h 473 2013-04-08 11:43:02Z rtlaka $
 *------------------------------------------------------------------------------
 */

#ifndef _osal_
#define _osal_

#include <stdint.h>

/* General types */
typedef uint8_t             boolean;
#define TRUE                1
#define FALSE               0
typedef int8_t              int8;
typedef int16_t             int16;
typedef int32_t             int32;
typedef uint8_t             uint8;
typedef uint16_t            uint16;
typedef uint32_t            uint32;
typedef int64_t             int64;
typedef uint64_t            uint64;
typedef float               float32;
typedef double              float64;

// define if debug printf is needed
//#define EC_DEBUG

#ifdef EC_DEBUG
#define EC_PRINT printf
#else
#define EC_PRINT(...) do {} while (0)
#endif

#ifndef PACKED
#define PACKED_BEGIN
#define PACKED  __attribute__((__packed__))
#define PACKED_END
#endif

#define OSAL_THREAD_HANDLE void *
#define OSAL_THREAD_FUNC void
#define OSAL_THREAD_FUNC_RT void

typedef struct
{
    uint32 sec;     /*< Seconds elapsed since the Epoch (Jan 1, 1970) */
    int32 usec;    /*< Microseconds elapsed since last second boundary */
} ec_timet;

typedef struct osal_timer
{
    ec_timet stop_time;
    uint64_t start_time;
} osal_timert;

void osal_timer_start(osal_timert * self, uint32 timeout_us);
boolean osal_timer_is_expired(osal_timert * self);
int osal_usleep(uint32 usec);
ec_timet osal_current_time(void);
int osal_thread_create(void *thandle, int stacksize, void *func, void *param);

#endif
