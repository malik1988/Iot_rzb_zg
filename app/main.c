/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "board.h"
#include "delay.h"

const uint8_t str_LOGO_RZB[] = { "\r\n"
		"	$$$$$$$\\ $$$$$$$$\$$$$$$$\\     \r\n"
		"	$$  __$$\\\\____$$  $$  __$$\\    \r\n"
		"	$$ |  $$ |   $$  /$$ |  $$ |   \r\n"
		"	$$$$$$$  |  $$  / $$$$$$$\\ |   \r\n"
		"	$$  __$$<  $$  /  $$  __$$\\    \r\n"
		"	$$ |  $$ |$$  /   $$ |  $$ |   \r\n"
		"	$$ |  $$ $$$$$$$$\\$$$$$$$  |   \r\n"
		"	\\__|  \\__\\________\\_______/    \r\n"
		"\r\n"

};

const uint8_t str_LOGO_ZG[] = { "\r\n"
		"	$$$$$$$$\\ $$$$$$\\    \r\n"
		"	\\____$$  $$  __$$\\  \r\n"
		"	    $$  /$$ /  \\__| \r\n"
		"	   $$  / $$ |$$$$\\  \r\n"
		"	  $$  /  $$ |\\_$$ | \r\n"
		"	 $$  /   $$ |  $$ | \r\n"
		"	$$$$$$$$\\ $$$$$$  | \r\n"
		" 	\\________|\\______/  \r\n"
		"\r\n" };

//公司LOGO
const uint8_t str_LOGO_USE[] = { "\r\n"
		"	$$\\   $$\\ $$$$$$\\ $$$$$$$$\\    \r\n"
		"	$$ |  $$ $$  __$$\\$$  _____|   \r\n"
		"	$$ |  $$ $$ /  \\__$$ |         \r\n"
		"	$$ |  $$ \\$$$$$$\ $$$$$\\       \r\n"
		"	$$ |  $$ |\\____$$\$$  __|      \r\n"
		"	$$ |  $$ $$\\   $$ $$ |         \r\n"
		"	\\$$$$$$  \\$$$$$$  $$$$$$$$\\    \r\n"
		"	 \\______/ \\______/\\________|   \r\n"

};
//未知
const uint8_t str_LOGO_UNKNOWN[] = { "UNKNOWN DEVICE" };

void System_Init() {
#if BOOT_LOADER
	//BOOT向量地址必须为0，默认值为0
#else
	//初始化中断向量0x20000地址必须和程序烧写的地址一致
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);
#endif
	SystemCoreClockUpdate();
	//配置优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	SysTick_Init();

}

//初始化板子硬件资源
void Board_Init() {
	printf_Init();
	Board_LEDInit(ALARM);
	Board_LEDInit(RUN);
	Board_PWM_Init();
	Board_Version_Init();
	//开启硬件程序读取保护
	//Board_Flash_Protect();

#if BOOT_LOADER

	printf("\n\r ############ 编译时间 ("__DATE__ " - " __TIME__ ")  ############ ");
	uint8_t *str_LOGO;
	if(Board_Is_RZB()) {
		str_LOGO=str_LOGO_RZB;
	}
	else {
		str_LOGO=str_LOGO_ZG;
	}
	printf("%s", str_LOGO);
#else//只有应用程序才执行
	//生成MAC地址
	Board_Mac_AutoGen();
	//初始化设备信息
	Board_DeviceInfo_Init();
	//打印设备信息
	Board_DeviceInfo_Print();

#if RZG	//网关才有以太网
	ETH_BSP_Config();
	//初始化UDP
	Dprintf("初始化以太网通讯服务.\n");
	LwIP_Init();//设置IP地址并初始化协议栈
	UDP_Server_Init();//初始化UDP服务器
#endif



#endif //ENDOF BOOT_LOADER
}
#if BOOT_LOADER
void Board_Boot_Service()
{
	if (IAP_Program_Check() == 0) {
		//蜂鸣器报警
		for (int i = 0; i < 10; i++)
		Board_PWM_On();
		printf("应用程序不存在！需要重新烧写程序！\n");
		//立即进入SHELL，需要设置应用程序
		Board_Shell_Entry(0);
	} else {
		//启动Shell
		Board_Shell_Entry(1);
	}

	//重新检测应用程序是否存在，如果不存在则死循环卡死在这里
	if (IAP_Program_Check() == 0) {
		printf("错误！应用启动 失败！\n");
		FOREVER()
		; //应用不存在，死循环
	} else {
		printf("应用程序开始。\n");

		IAP_Program_Run();
	}
}
#endif	//ENDOF BOOT_LOADER
int main() {
	System_Init(); //系统初始化
	Board_Init(); //板子初始化
#if BOOT_LOADER
	Board_Boot_Service();
#else
	stack_main();
#endif
}

/****************************END OF FILE****/
