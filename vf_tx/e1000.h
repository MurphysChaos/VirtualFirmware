#ifndef E1000_H
#define E1000_H

#include <stdint.h>

/* Admin Queue registers */
#define E1000_PF_ATQBAH                 0x00006000
#define E1000_PF_ATQBAH_ATQBAH_SHIFT    0
#define E1000_PF_ATQBAH_ATQBAH_MASK     (0xFFFFFFFF << E1000_PF_ATQBAH_ATQBAH_SHIFT)
#define E1000_PF_ATQBAL                 0x00006004
#define E1000_PF_ATQBAL_ATQBAL_SHIFT    0
#define E1000_PF_ATQBAL_ATQBAL_MASK     (0xFFFFFFFF << E1000_PF_ATQBAL_ATQBAL_SHIFT)
#define E1000_PF_ATQH                   0x00006008
#define E1000_PF_ATQH_ATQH_SHIFT        0
#define E1000_PF_ATQH_ATQH_MASK         (0x3FF << E1000_PF_ATQH_ATQH_SHIFT)
#define E1000_PF_ATQLEN                 0x0000600C
#define E1000_PF_ATQLEN_ATQLEN_SHIFT    0
#define E1000_PF_ATQLEN_ATQLEN_MASK     (0x3FF << E1000_PF_ATQLEN_ATQLEN_SHIFT)
#define E1000_PF_ATQLEN_ATQENABLE_SHIFT 31
#define E1000_PF_ATQLEN_ATQENABLE_MASK  (0x1 << E1000_PF_ATQLEN_ATQENABLE_SHIFT)
#define E1000_PF_ATQT                   0x00006010
#define E1000_PF_ATQT_ATQT_SHIFT        0
#define E1000_PF_ATQT_ATQT_MASK         (0x3FF << E1000_PF_ATQT_ATQT_SHIFT)


enum e1000_admin_queue_cmd {
        e1000_aqc_get_version           = 0x0001,
        e1000_aqc_driver_heartbeat      = 0x0002,
        e1000_aqc_echo                  = 0x0003,
};

/*
 * Basic adminq descriptor
 */
struct e1000_aq_desc {
        uint16_t flags;
        uint16_t opcode;
        uint16_t datalen;
        uint16_t retval;
        uint32_t cookie_high;
        uint32_t cookie_low;
        uint32_t param0;
        uint32_t param1;
        uint32_t addr_high;
        uint32_t addr_low;
};

/* command flags and offsets */
#define E1000_AQ_FLAG_DD_OFF_SHIFT      0
#define E1000_AQ_FLAG_CMP_OFF_SHIFT     1
#define E1000_AQ_FLAG_ERR_OFF_SHIFT     2
#define E1000_AQ_FLAG_VFE_OFF_SHIFT     3
#define E1000_AQ_FLAG_LB_OFF_SHIFT      4
#define E1000_AQ_FLAG_RD_OFF_SHIFT      0xA
#define E1000_AQ_FLAG_VFC_OFF_SHIFT     0xB
#define E1000_AQ_FLAG_BUF_OFF_SHIFT     0xC
#define E1000_AQ_FLAG_SI_OFF_SHIFT      0xD
#define E1000_AQ_FLAG_EI_OFF_SHIFT      0xE
#define E1000_AQ_FLAG_FE_OFF_SHIFT      0xF

#define E1000_AQ_FLAG_DD                (1 << E1000_AQ_FLAG_DD_OFF_SHIFT)
#define E1000_AQ_FLAG_CMP               (1 << E1000_AQ_FLAG_CMP_OFF_SHIFT)
#define E1000_AQ_FLAG_ERR               (1 << E1000_AQ_FLAG_ERR_OFF_SHIFT)
#define E1000_AQ_FLAG_VFE               (1 << E1000_AQ_FLAG_VFE_OFF_SHIFT)
#define E1000_AQ_FLAG_LB                (1 << E1000_AQ_FLAG_LB_OFF_SHIFT)
#define E1000_AQ_FLAG_RD                (1 << E1000_AQ_FLAG_RD_OFF_SHIFT)
#define E1000_AQ_FLAG_VFC               (1 << E1000_AQ_FLAG_VFC_OFF_SHIFT)
#define E1000_AQ_FLAG_BUF               (1 << E1000_AQ_FLAG_BUF_OFF_SHIFT)
#define E1000_AQ_FLAG_SI                (1 << E1000_AQ_FLAG_SI_OFF_SHIFT)
#define E1000_AQ_FLAG_EI                (1 << E1000_AQ_FLAG_EI_OFF_SHIFT)
#define E1000_AQ_FLAG_FE                (1 << E1000_AQ_FLAG_FE_OFF_SHIFT)

enum e1000_admin_queue_err {
        E1000_AQ_RC_OK          = 0,
        E1000_AQ_RC_ENOSYS      = 1,
};


#endif
