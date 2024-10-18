/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * Headerfile for oshw.c
 */

#ifndef _oshw_
#define _oshw_

#ifdef __cplusplus
extern "C"
{
#endif

#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatmain.h"

#define htons(A) ((((uint16)(A) & 0xff00) >> 8) | \
                (((uint16)(A) & 0x00ff) << 8))
#define htonl(A) ((((uint32)(A) & 0xff000000) >> 24) | \
                (((uint32)(A) & 0x00ff0000) >> 8)  | \
                (((uint32)(A) & 0x0000ff00) << 8)  | \
                (((uint32)(A) & 0x000000ff) << 24))

uint16 oshw_htons(uint16 host);
uint16 oshw_ntohs(uint16 network);

ec_adaptert * oshw_find_adapters(void);
void oshw_free_adapters(ec_adaptert * adapter);

#ifdef __cplusplus
}
#endif

#endif
