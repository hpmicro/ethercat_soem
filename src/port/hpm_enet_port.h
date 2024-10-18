#ifndef HPM_ENET_PORT_H
#define HPM_ENET_PORT_H

#include "board.h"

#if defined(RGMII) && RGMII
#define ENET_INF_TYPE       enet_inf_rgmii
#define ENET                BOARD_ENET_RGMII
#else
#define ENET_INF_TYPE       enet_inf_rmii
#define ENET                BOARD_ENET_RMII
#endif

#define ENET_TX_BUFF_COUNT  (1U)
#define ENET_RX_BUFF_COUNT  (1U)
#define ENET_RX_BUFF_SIZE   ENET_MAX_FRAME_SIZE
#define ENET_TX_BUFF_SIZE   ENET_MAX_FRAME_SIZE

/* MAC ADDRESS */
//#define MAC_ADDR0   0x98
//#define MAC_ADDR1   0x2C
//#define MAC_ADDR2   0xBC
//#define MAC_ADDR3   0xB1
//#define MAC_ADDR4   0x9F
//#define MAC_ADDR5   0x17

#define MAC_ADDR0   0x00
#define MAC_ADDR1   0x80
#define MAC_ADDR2   0xE1
#define MAC_ADDR3   0x00
#define MAC_ADDR4   0x00
#define MAC_ADDR5   0x00

int bfin_emac_init(void);
int bfin_emac_send (void *packet, int length);
int bfin_emac_recv (uint8_t * packet, size_t size);
void bfin_polling_link_status(void);
bool bfin_get_link_status(void);

#endif
