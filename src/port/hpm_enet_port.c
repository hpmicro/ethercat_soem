#include "hpm_enet_port.h"
#include "hpm_clock_drv.h"
#include "hpm_enet_drv.h"
#include "hpm_enet_phy_common.h"
#include "hpm_otp_drv.h"
#include "board.h"

ATTR_PLACE_AT_FAST_RAM_INIT_WITH_ALIGNMENT(ENET_SOC_DESC_ADDR_ALIGNMENT)
__RW enet_rx_desc_t dma_rx_desc_tab[ENET_RX_BUFF_COUNT]; /* Ethernet Rx DMA Descriptor */

ATTR_PLACE_AT_FAST_RAM_INIT_WITH_ALIGNMENT(ENET_SOC_DESC_ADDR_ALIGNMENT)
__RW enet_tx_desc_t dma_tx_desc_tab[ENET_TX_BUFF_COUNT]; /* Ethernet Tx DMA Descriptor */

ATTR_PLACE_AT_FAST_RAM_INIT_WITH_ALIGNMENT(ENET_SOC_BUFF_ADDR_ALIGNMENT)
__RW uint8_t rx_buff[ENET_RX_BUFF_COUNT][ENET_RX_BUFF_SIZE]; /* Ethernet Receive Buffer */

ATTR_PLACE_AT_FAST_RAM_INIT_WITH_ALIGNMENT(ENET_SOC_BUFF_ADDR_ALIGNMENT)
__RW uint8_t tx_buff[ENET_TX_BUFF_COUNT][ENET_TX_BUFF_SIZE]; /* Ethernet Transmit Buffer */

enet_desc_t desc;
uint8_t mac[ENET_MAC];

ATTR_WEAK void enet_get_mac_address(uint8_t *mac)
{
    bool invalid = true;

    uint32_t uuid[(ENET_MAC + (ENET_MAC - 1)) / sizeof(uint32_t)];

    for (int i = 0; i < ARRAY_SIZE(uuid); i++)
    {
        uuid[i] = otp_read_from_shadow(OTP_SOC_UUID_IDX + i);
        if (uuid[i] != 0xFFFFFFFFUL && uuid[i] != 0)
        {
            invalid = false;
        }
    }

    if (invalid == true)
    {
        memcpy(mac, &uuid, ENET_MAC);
    }
    else
    {
        mac[0] = MAC_ADDR0;
        mac[1] = MAC_ADDR1;
        mac[2] = MAC_ADDR2;
        mac[3] = MAC_ADDR3;
        mac[4] = MAC_ADDR4;
        mac[5] = MAC_ADDR5;
    }
}

/*---------------------------------------------------------------------*
 * Initialization
 *---------------------------------------------------------------------*/
static hpm_stat_t enet_init(ENET_Type *ptr)
{
    enet_int_config_t int_config = {.int_enable = 0, .int_mask = 0};
    enet_mac_config_t enet_config;
#if defined(RGMII) && RGMII
#if defined(__USE_DP83867) && __USE_DP83867
    dp83867_config_t phy_config;
#else
    rtl8211_config_t phy_config;
#endif
#else
#if defined(__USE_DP83848) && __USE_DP83848
    dp83848_config_t phy_config;
#else
    rtl8201_config_t phy_config;
#endif
#endif

    /* Initialize td, rd and the corresponding buffers */
    memset((uint8_t *)dma_tx_desc_tab, 0x00, sizeof(dma_tx_desc_tab));
    memset((uint8_t *)dma_rx_desc_tab, 0x00, sizeof(dma_rx_desc_tab));
    memset((uint8_t *)rx_buff, 0x00, sizeof(rx_buff));
    memset((uint8_t *)tx_buff, 0x00, sizeof(tx_buff));

    desc.tx_desc_list_head = (enet_tx_desc_t *)core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)dma_tx_desc_tab);
    desc.rx_desc_list_head = (enet_rx_desc_t *)core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)dma_rx_desc_tab);

    desc.tx_buff_cfg.buffer = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)tx_buff);
    desc.tx_buff_cfg.count = ENET_TX_BUFF_COUNT;
    desc.tx_buff_cfg.size = ENET_TX_BUFF_SIZE;

    desc.rx_buff_cfg.buffer = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)rx_buff);
    desc.rx_buff_cfg.count = ENET_RX_BUFF_COUNT;
    desc.rx_buff_cfg.size = ENET_RX_BUFF_SIZE;

    /* Get MAC address */
    enet_get_mac_address(mac);

    /* Set mac0 address */
    enet_config.mac_addr_high[0] = mac[5] << 8 | mac[4];
    enet_config.mac_addr_low[0] = mac[3] << 24 | mac[2] << 16 | mac[1] << 8 | mac[0];
    enet_config.valid_max_count = 1;

    /* Set DMA PBL */
    enet_config.dma_pbl = board_get_enet_dma_pbl(ENET);

    /* Initialize enet controller */
    enet_controller_init(ptr, ENET_INF_TYPE, &desc, &enet_config, &int_config);

/* Initialize phy */
#if defined(RGMII) && RGMII
#if defined(__USE_DP83867) && __USE_DP83867
    dp83867_reset(ptr);
    dp83867_basic_mode_default_config(ptr, &phy_config);
    if (dp83867_basic_mode_init(ptr, &phy_config) == true)
    {
#else
    rtl8211_reset(ptr);
    rtl8211_basic_mode_default_config(ptr, &phy_config);
    if (rtl8211_basic_mode_init(ptr, &phy_config) == true)
    {
#endif
#else
#if defined(__USE_DP83848) && __USE_DP83848
    dp83848_reset(ptr);
    dp83848_basic_mode_default_config(ptr, &phy_config);
    if (dp83848_basic_mode_init(ptr, &phy_config) == true)
    {
#else
    rtl8201_reset(ptr);
    rtl8201_basic_mode_default_config(ptr, &phy_config);
    if (rtl8201_basic_mode_init(ptr, &phy_config) == true)
    {
#endif
#endif
        printf("Enet phy init passes !\n");
        return status_success;
    }
    else
    {
        printf("Enet phy init fails !\n");
        return status_fail;
    }
}

void ethcat_init(void)
{
    /* Initialize GPIOs */
    board_init_enet_pins(ENET);

    /* Reset an enet PHY */
    board_reset_enet_phy(ENET);
#if defined(RGMII) && RGMII
    /* Set RGMII clock delay */
    board_init_enet_rgmii_clock_delay(ENET);
#else
    /* Set RMII reference clock */
    board_init_enet_rmii_reference_clock(ENET, BOARD_ENET_RMII_INT_REF_CLK);
    printf("Reference Clock: %s\n", BOARD_ENET_RMII_INT_REF_CLK ? "Internal Clock" : "External Clock");
#endif

    /* Initialize MAC and DMA */
    if (enet_init(ENET) == 0)
    {
    }
    else
    {
        printf("Enet initialization fails !!!\n");
        while (1)
        {
        }
    }
}

int bfin_emac_init(void)
{
    ethcat_init();
    return 0;
}

ATTR_RAMFUNC
int bfin_emac_send(void *packet, int length)
{
    uint8_t *buffer;
    __IO enet_tx_desc_t *dma_tx_desc;
    uint16_t frame_length = 0;
    uint32_t buffer_offset = 0;
    uint32_t bytes_left_to_copy = 0;
    uint32_t payload_offset = 0;
    enet_tx_desc_t *tx_desc_list_cur = (enet_tx_desc_t*)sys_address_to_core_local_mem(BOARD_RUNNING_CORE, (uint32_t)desc.tx_desc_list_cur);

    dma_tx_desc = tx_desc_list_cur;
    buffer = (uint8_t *)sys_address_to_core_local_mem(BOARD_RUNNING_CORE, dma_tx_desc->tdes2_bm.buffer1);
    buffer_offset = 0;

    bytes_left_to_copy = length;
    payload_offset = 0;
    if (dma_tx_desc->tdes0_bm.own != 0)
    {
        return -1;
    }
    /* Check if the length of data to copy is bigger than Tx buffer size*/
    while ((bytes_left_to_copy + buffer_offset) > ENET_TX_BUFF_SIZE)
    {
        /* Copy data to Tx buffer*/
        memcpy((uint8_t *)((uint8_t *)buffer + buffer_offset),
               (uint8_t *)((uint8_t *)packet + payload_offset),
               ENET_TX_BUFF_SIZE - buffer_offset);

        /* Point to next descriptor */
        dma_tx_desc = (enet_tx_desc_t *)sys_address_to_core_local_mem(BOARD_RUNNING_CORE, (uint32_t)dma_tx_desc->tdes3_bm.next_desc);

        /* Check if the buffer is available */
        if (dma_tx_desc->tdes0_bm.own != 0)
        {
            return -1;
        }

        buffer = (uint8_t *)sys_address_to_core_local_mem(BOARD_RUNNING_CORE, dma_tx_desc->tdes2_bm.buffer1);

        bytes_left_to_copy = bytes_left_to_copy - (ENET_TX_BUFF_SIZE - buffer_offset);
        payload_offset = payload_offset + (ENET_TX_BUFF_SIZE - buffer_offset);
        frame_length = frame_length + (ENET_TX_BUFF_SIZE - buffer_offset);
        buffer_offset = 0;
    }

    /* pass payload to buffer */
    tx_desc_list_cur = (enet_tx_desc_t*)sys_address_to_core_local_mem(BOARD_RUNNING_CORE, (uint32_t)desc.tx_desc_list_cur);
    tx_desc_list_cur->tdes2_bm.buffer1 = core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)packet);
    buffer_offset = buffer_offset + bytes_left_to_copy;
    frame_length = frame_length + bytes_left_to_copy;

    /* Prepare transmit descriptors to give to DMA*/
    frame_length += 4;
    enet_prepare_transmission_descriptors(ENET, &tx_desc_list_cur, frame_length, desc.tx_buff_cfg.size);
    return 0;
}

ATTR_RAMFUNC
int bfin_emac_recv(uint8_t *packet, size_t size)
{
    uint32_t sta = 0;
    uint32_t len;
    uint8_t *buffer;

    enet_frame_t frame = {0, 0, 0};
    enet_rx_desc_t *dma_rx_desc;
    uint32_t buffer_offset = 0;
    uint32_t payload_offset = 0;
    uint32_t bytes_left_to_copy = 0;
    uint32_t i = 0;
    enet_rx_desc_t *rx_desc_list_cur = (enet_rx_desc_t *)sys_address_to_core_local_mem(BOARD_RUNNING_CORE, (uint32_t)desc.rx_desc_list_cur);
    /* Check and get a received frame */
#if 0
    frame = enet_get_received_frame_interrupt(&rx_desc_list_cur, &desc.rx_frame_info, ENET_RX_BUFF_COUNT);
#else
    sta = enet_check_received_frame(&rx_desc_list_cur, &desc.rx_frame_info);
    if (1 == sta)
    {
        frame = enet_get_received_frame(&rx_desc_list_cur, &desc.rx_frame_info);
        /* Obtain the size of the packet and put it into the "len" variable. */
        len = frame.length;
        buffer = (uint8_t *)sys_address_to_core_local_mem(BOARD_RUNNING_CORE, frame.buffer);
        if (len <= 0)
            return -1;
        dma_rx_desc = frame.rx_desc;
        buffer_offset = 0;

        bytes_left_to_copy = len;
        payload_offset = 0;
        /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size*/
        while ((bytes_left_to_copy + buffer_offset) > ENET_RX_BUFF_SIZE)
        {
            /* Copy data to pbuf*/
            memcpy((uint8_t *)((uint8_t *)packet + payload_offset), (uint8_t *)((uint8_t *)buffer + buffer_offset), (ENET_RX_BUFF_SIZE - buffer_offset));

            /* Point to next descriptor */
            dma_rx_desc = (enet_rx_desc_t *)(dma_rx_desc->rdes3_bm.next_desc);
            buffer = (uint8_t *)sys_address_to_core_local_mem(BOARD_RUNNING_CORE, dma_rx_desc->rdes2_bm.buffer1);

            bytes_left_to_copy = bytes_left_to_copy - (ENET_RX_BUFF_SIZE - buffer_offset);
            payload_offset = payload_offset + (ENET_RX_BUFF_SIZE - buffer_offset);
            buffer_offset = 0;
        }
        /* pass the buffer to pbuf */
        // packet = buffer;
        buffer_offset = buffer_offset + bytes_left_to_copy;
        memcpy((uint8_t *)((uint8_t *)packet), (uint8_t *)((uint8_t *)buffer), buffer_offset);
        /* Release descriptors to DMA */
        dma_rx_desc = frame.rx_desc;

        /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
        for (i = 0; i < desc.rx_frame_info.seg_count; i++)
        {
            dma_rx_desc->rdes0_bm.own = 1;
            dma_rx_desc = (enet_rx_desc_t *)(dma_rx_desc->rdes3_bm.next_desc);
        }

        /* Clear Segment_Count */
        desc.rx_frame_info.seg_count = 0;
        return buffer_offset;
    }
#endif
    return -1;
}

static enet_phy_status_t last_status;

void bfin_polling_link_status(void)
{
    enet_phy_status_t status = {0};

    enet_line_speed_t line_speed[] = {enet_line_speed_10mbps, enet_line_speed_100mbps, enet_line_speed_1000mbps};
    char *speed_str[] = {"10Mbps", "100Mbps", "1000Mbps"};
    char *duplex_str[] = {"Half duplex", "Full duplex"};

#if defined(RGMII) && RGMII
    #if defined(__USE_DP83867) && __USE_DP83867
        dp83867_get_phy_status(ENET, &status);
    #else
        rtl8211_get_phy_status(ENET, &status);
    #endif
#else
    #if defined(__USE_DP83848) && __USE_DP83848
        dp83848_get_phy_status(ENET, &status);
    #else
        rtl8201_get_phy_status(ENET, &status);
    #endif
#endif

    if (memcmp(&last_status, &status, sizeof(enet_phy_status_t)) != 0) {
        memcpy(&last_status, &status, sizeof(enet_phy_status_t));
        if (status.enet_phy_link) {
            printf("Link Status: Up\n");
            printf("Link Speed:  %s\n", speed_str[status.enet_phy_speed]);
            printf("Link Duplex: %s\n", duplex_str[status.enet_phy_duplex]);
            enet_set_line_speed(ENET, line_speed[status.enet_phy_speed]);
            enet_set_duplex_mode(ENET, status.enet_phy_duplex);
        } else {
            printf("Link Status: Down\n");
        }
    }
}

bool bfin_get_link_status(void)
{
    return last_status.enet_phy_link;
}
