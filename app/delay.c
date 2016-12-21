#include "delay.h"
static __IO uint32_t uTickMs; //MCU Tick数

void TimingInc() {
	uTickMs++;
}
//将系统Tick的值转成us为单位的时间值
uint32_t GetUsFromTicks(uint32_t ticks) {
	uint32_t ret;

	if (120000000 == SystemCoreClock) { //优化计算
		ret = ticks / 120;
	} else { //可能计算存在误差
		ret = 1000 * ticks / SYSTICK_MAX_VALUE; //us
	}
	return ret;
}
//获取us为单位的时间值，最大表示2^32/1000/1000秒
uint32_t GetTimeUS() {
	uint32_t tms, ticks; //保存ms值
	ticks = GetTick();
	tms = uTickMs;

	return GetUsFromTicks(ticks) + tms * 1000;
}
//获取ms为单位的时间值
uint32_t GetTimeMS() {
	return uTickMs;
}

//系统Tick数，该数/(SYSTICK_MAX_VALUE)即为ms数
uint32_t GetTick() {
	return SYSTICK_MAX_VALUE-SysTick->VAL;//VAL为倒计数
}

//使用STM32的SysTick进行精确时间延迟
void SysTick_Init() {
	//配置成1ms一个Tick
	SysTick_Config(SYSTICK_MAX_VALUE);
	//初始化uTick
	uTickMs = 0;
}

//延迟毫秒，不可重复调用，阻塞方式，不可放中断中执行
void delayMS(uint32_t ms) {
	uint32_t timingdelay;
	timingdelay = uTickMs + ms;
	while (uTickMs < timingdelay) {
	}
}

//系统时钟中断服务程序，中断优先级最高
void SysTick_Handler(void) {
	TimingInc();
}
//延迟US，不可嵌套调用，阻塞方式，不可放入中断
void delayUS(uint32_t us){
	uint32_t t;
	t=GetTimeUS()+us;
	while(GetTimeUS()<t){

	}
}
