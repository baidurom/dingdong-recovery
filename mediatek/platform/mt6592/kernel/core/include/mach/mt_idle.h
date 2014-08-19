#ifndef _MT_IDLE_H
#define _MT_IDLE_H

extern void enable_dpidle_by_bit(int id);
extern void disable_dpidle_by_bit(int id);
extern void enable_soidle_by_bit(int id);
extern void disable_soidle_by_bit(int id);

#endif
