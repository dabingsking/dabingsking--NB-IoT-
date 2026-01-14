#ifndef __FLAG_H
#define __FLAG_H

#include "sys.h"

#define KEY1_PRESS_FLAG		0x0001
#define KEY2_PRESS_FLAG		0x0002

#define SYS_LDR_FLAG		0x0001

extern uint16_t key_flag, sys_flag;

void flag_init(void);
void flag_set(uint16_t *flag,uint16_t bit_mask);
void flag_clr(uint16_t *flag,uint16_t bit_mask);
uint8_t flag_get(uint16_t *flag,uint16_t bit_mask);

#endif
