#ifndef __TIM_INNIT_H
#define __TIM_INNIT_H

#include "sys_user.h"
#include "stm32f4xx_conf.h"
#include "lwip/opt.h"

void enable_timer(void);
void disable_timer(void);
void timer_init(void);
#endif
