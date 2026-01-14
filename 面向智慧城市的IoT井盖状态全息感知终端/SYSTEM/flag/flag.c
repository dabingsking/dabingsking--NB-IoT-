#include "flag.h"

uint16_t key_flag = 0x0000, sys_flag = 0x0000;

void flag_init(void)
{
	key_flag = 0x0000;
	sys_flag = 0x0000;
}

void flag_set(uint16_t *flag,uint16_t bit_mask)
{
	*flag = (*flag) | bit_mask;
}

void flag_clr(uint16_t *flag,uint16_t bit_mask)
{
	*flag = (*flag) & (~bit_mask);
}

uint8_t flag_get(uint16_t *flag,uint16_t bit_mask)
{
	if(((*flag) & bit_mask) == bit_mask) return 1;
	else return 0;
}




