
#ifndef DELAY_H
#define DELAY_H

#include "stm32f2xx.h"


#define SYSTICK_MAX_VALUE 120000 //系统Tick计数器的最大值，等于SystemCoreClock / SYSTEM_CLK_PRESCALE
//系统Tick频率值，1000为1000Hz，则为1ms一次计时。
#define SYSTEM_CLK_PRESCALE 1000

#define DELTA_TICKS(now,pre)   (now>pre? now-pre : now+120000-pre)

void TimingInc();
void delayMS(uint32_t ms);
uint32_t GetTick();
uint32_t GetUsFromTicks(uint32_t ticks);
uint32_t GetTimeUS();




#endif //ENDOF DELAY_H

