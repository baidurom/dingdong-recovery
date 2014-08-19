#ifndef _SYSTRAKCER_H
#define _SYSTRAKCER_H

#define BUS_DBG_CON (BUS_DBG_TRACKER_BASE + 0x0000)
#define BUS_DBG_TIMER_CON (BUS_DBG_TRACKER_BASE + 0x0004)
#define BUS_DBG_TIMER (BUS_DBG_TRACKER_BASE + 0x0008)
#define BUS_DBG_WP (BUS_DBG_TRACKER_BASE + 0x000C)
#define BUS_DBG_WP_MASK (BUS_DBG_TRACKER_BASE + 0x0010)
#define BUS_DBG_AR_TRACK_L(__n) (BUS_DBG_TRACKER_BASE + 0x0100 + 8 * (__n))
#define BUS_DBG_AR_TRACK_H(__n) (BUS_DBG_TRACKER_BASE + 0x0104 + 8 * (__n))
#define BUS_DBG_AR_TRANS_TID(__n) (BUS_DBG_TRACKER_BASE + 0x0180 + 4 * (__n))
#define BUS_DBG_AW_TRACK_L(__n) (BUS_DBG_TRACKER_BASE + 0x0200 + 8 * (__n))
#define BUS_DBG_AW_TRACK_H(__n) (BUS_DBG_TRACKER_BASE + 0x0204 + 8 * (__n))
#define BUS_DBG_AW_TRANS_TID(__n) (BUS_DBG_TRACKER_BASE + 0x0280 + 4 * (__n))

#define TRACKER_DBG (0)
#define DEFAULT_BUS_MHZ (266)
#define DEFAULT_CONTROL_VAL (0x00004007)
#define BUS_DBG_CON_IRQ_EN  (0x00000070)
#define BUS_DBG_CON_WP_EN   (0x00000008)



extern int enable_systracker(void);
extern int disable_systracker(void);
//extern int enable_systracker(int use_irq_mode, int timeout_ms,  \
//                unsigned int  mask, unsigned int  val);
extern int set_watchpoint(unsigned int watch_addr, unsigned int addr_mask);
static inline unsigned int extract_n2mbits(unsigned int input,int n, int m)
{
/*
 * 1. ~0 = 1111 1111 1111 1111 1111 1111 1111 1111
 * 2. ~0 << (m - n + 1) = 1111 1111 1111 1111 1100 0000 0000 0000
 * // assuming we are extracting 14 bits, the +1 is added for inclusive selection
 * 3. ~(~0 << (m - n + 1)) = 0000 0000 0000 0000 0011 1111 1111 1111
 * */
        int value;
        int mask;
        if (n < m) {
            n = n + m;
            m = n - m;
            n = n - m;
        }
        mask = ~(~0 << (m - n + 1));
        value = (input >> n) & mask;
        return ;
}

#endif

