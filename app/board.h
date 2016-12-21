/**
 *@file board.c
 *@brief 板硬件接口定义(MCU-STM32F207ZG)
 *@author lxm
 *@date 2016.10.13
 *@version 0.1.0
 *@par Copyright (c):
 *      XXX公司
 *@par 修改日志
 *      0.1.0 xx 2016.10.11 创建文件
 *@details 详细硬件清单请见board_config.h
 **/
#ifndef BOARD_H
#define BOARD_H

#include "stm32f2xx.h"
#include "board_config.h"

/*------------------COM(调试串口)定义--------------------------*/
typedef enum {
	COM1 = 0, COM2 = 1, COMn //最后一个用于计数总个数，请勿修改，并且保证ENUM中第一个值为0
} COM_TypeDef;

/**
 * @brief Definition for COM port1, connected to USART3
 */
#define EVAL_COM1                        USART1
#define EVAL_COM1_STR                    "USART1"
#define EVAL_COM1_CLK                    RCC_APB2Periph_USART1
#define EVAL_COM1_TX_PIN                 GPIO_Pin_9
#define EVAL_COM1_TX_GPIO_PORT           GPIOA
#define EVAL_COM1_TX_GPIO_CLK            RCC_AHB1Periph_GPIOA
#define EVAL_COM1_TX_SOURCE              GPIO_PinSource9
#define EVAL_COM1_TX_AF                  GPIO_AF_USART1
#define EVAL_COM1_RX_PIN                 GPIO_Pin_10
#define EVAL_COM1_RX_GPIO_PORT           GPIOA
#define EVAL_COM1_RX_GPIO_CLK            RCC_AHB1Periph_GPIOA
#define EVAL_COM1_RX_SOURCE              GPIO_PinSource10
#define EVAL_COM1_RX_AF                  GPIO_AF_USART1
#define EVAL_COM1_IRQn                   USART1_IRQn

/**
 * @brief Definition for COM port2, connected to USART3
 */

#if (RZG||RZB||BOOT_LOADER)		//RZG/RZB都使用  COM2--USART4作为串口打印
#define EVAL_COM2                        UART4
#define EVAL_COM2_STR                    "UART4"
#define EVAL_COM2_CLK                    RCC_APB1Periph_UART4
#define EVAL_COM2_TX_PIN                 GPIO_Pin_10	//PC10
#define EVAL_COM2_TX_GPIO_PORT           GPIOC
#define EVAL_COM2_TX_GPIO_CLK            RCC_AHB1Periph_GPIOC
#define EVAL_COM2_TX_SOURCE              GPIO_PinSource10
#define EVAL_COM2_TX_AF                  GPIO_AF_UART4
#define EVAL_COM2_RX_PIN                 GPIO_Pin_11	//PC11
#define EVAL_COM2_RX_GPIO_PORT           GPIOC
#define EVAL_COM2_RX_GPIO_CLK            RCC_AHB1Periph_GPIOC
#define EVAL_COM2_RX_SOURCE              GPIO_PinSource11
#define EVAL_COM2_RX_AF                  GPIO_AF_UART4
#define EVAL_COM2_IRQn                   UART4_IRQn

//串口使能引脚
#define EVAL_COM2_EN_PIN				GPIO_Pin_9	//PA9
#define EVAL_COM2_EN_PORT				GPIOA
#define EVAL_COM2_EN_CLKEN()			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE)

#else									//COM2--USART3
#define EVAL_COM2                        USART3
#define EVAL_COM2_STR                    "USART3"
#define EVAL_COM2_CLK                    RCC_APB1Periph_USART3
#define EVAL_COM2_TX_PIN                 GPIO_Pin_10
#define EVAL_COM2_TX_GPIO_PORT           GPIOB
#define EVAL_COM2_TX_GPIO_CLK            RCC_AHB1Periph_GPIOB
#define EVAL_COM2_TX_SOURCE              GPIO_PinSource10
#define EVAL_COM2_TX_AF                  GPIO_AF_USART3
#define EVAL_COM2_RX_PIN                 GPIO_Pin_11
#define EVAL_COM2_RX_GPIO_PORT           GPIOB
#define EVAL_COM2_RX_GPIO_CLK            RCC_AHB1Periph_GPIOB
#define EVAL_COM2_RX_SOURCE              GPIO_PinSource11
#define EVAL_COM2_RX_AF                  GPIO_AF_USART3
#define EVAL_COM2_IRQn                   USART3_IRQn
#endif

/*------------------LED定义--------------------------*/
typedef enum {
	ALARM = 0, //告警指示灯
	RUN, //系统运行指示灯

	LEDn //最后一个用于计数LED总个数，请勿修改，并且保证ENUM中第一个值为0
} Led_TypeDef;

#define LED_ALARM_PIN                    GPIO_Pin_7	//PB7
#define LED_ALARM_PORT                   GPIOB
#define LED_ALARM_CLK                    RCC_AHB1Periph_GPIOB

#define LED_RUN_PIN						 GPIO_Pin_8	//PB8
#define LED_RUN_PORT					 GPIOB
#define LED_RUN_CLK						 RCC_AHB1Periph_GPIOB

/*------------------蜂鸣器定义--------------------------*/
#define PWM_PIN 						GPIO_Pin_9	//PE9
#define PWM_PORT 						GPIOE
#define PWM_CLK							RCC_AHB1Periph_GPIOE
#define PWM_PIN_SOURCE                  GPIO_PinSource9

#define PWM_LOW()  						GPIO_ResetBits(PWM_PORT, PWM_PIN)
#define PWM_HIGH()  					GPIO_SetBits(PWM_PORT, PWM_PIN)

#define PWM_USE_TIM  					1	//使用定时器TIM1产生PWM驱动蜂鸣器
#define PWM_TIM_CLK_EN()     			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE)// PE9硬件与TIM1_CH1绑定
#define PWM_TIM_PIN_CONNECT()			GPIO_PinAFConfig(PWM_PORT, GPIO_PinSource9, GPIO_AF_TIM1)
#define PWM_TIM  						TIM2
#define PWM_TIM_IRQn					TIM2_IRQn
#define PWM_TIM_IRQ_PRIO_EMPTION 		2
#define PWM_TIM_IRQ_PRIO_SUB			1
#define Board_PWM_IRQHandler			TIM2_IRQHandler

/*------------------RTC定义--------------------------*/
#define RTC_IRQ_LINE_TS				EXTI_Line21	//RTC Tamper和Stamper中断连接线为12
#define RTC_IRQ_CHANNEL_TS			TAMP_STAMP_IRQn //RTC LIN12中断通道
#define RTC_IRQHandler_TS			TAMP_STAMP_IRQHandler//RTC Tamper和STamper中断
#define RTC_IRQ_PRIO_EMPTION_TS   	5	//主优先级,抢占
#define RTC_IRQ_PRIO_SUB_TS			6	//次优先级,时间片
#define RTC_IRQ_LINE_ALARM			EXTI_Line17 //RTC Alarm中断线连接是17
#define RTC_IRQ_CHANNEL_ALARM		RTC_Alarm_IRQn//RTC ALARM中断通道
#define RTC_IRQHandler_ALARM		RTC_Alarm_IRQHandler//RTC Tamper和STamper中断
#define RTC_IRQ_PRIO_EMPTION_ALARM  5	//主优先级,抢占
#define RTC_IRQ_PRIO_SUB_ALARM		7	//次优先级,时间片
/*------------------内部Flash定义--------------------------*/
/*------FLASH Sector 表 （总共512K）-----*/
#define FLASH_BASE_SECTOR_0		    (FLASH_BASE+0X0)		//16k
#define FLASH_BASE_SECTOR_1		    (FLASH_BASE+0X4000)		//16k
#define FLASH_BASE_SECTOR_2		    (FLASH_BASE+0X8000)		//16k
#define FLASH_BASE_SECTOR_3		    (FLASH_BASE+0Xc000)		//16k
#define FLASH_BASE_SECTOR_4		    (FLASH_BASE+0X10000)	//64k
#define FLASH_BASE_SECTOR_5		    (FLASH_BASE+0X20000)	//128k
#define FLASH_BASE_SECTOR_6		    (FLASH_BASE+0X40000)	//128k
#define FLASH_BASE_SECTOR_7		    (FLASH_BASE+0X60000)	//128k

//地址必须保证不会与程序存储地址有交叉,先取0x40000=256*1024=256k。
//地址具体的sector参考芯片手册
#define USER_FLASH_BASE  			(FLASH_BASE+0x40000)  //sector6
#define USER_FLASH_SIZE 			0x20000				//sector6 大小为128K
#define USER_FLASH_SECTOR			FLASH_Sector_6

#define USER_FLASH_MAC_BASE			USER_FLASH_BASE		//MAC地址的首地址
/*------------------MCU设备ID----------------------*/
#define MCU_ID_BASE					0x1FFF7A10			//ID基地址
/*------------------函数声明--------------------------*/
void Board_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct);
void Board_LEDInit(Led_TypeDef Led);
void Board_LedOn(Led_TypeDef Led);
void Board_LedOff(Led_TypeDef Led);
void Board_PWM_Init();
void Board_PWM_On();
#if PWM_USE_TIM
void Board_PWM_Off();
ErrorStatus Board_PWM_SetCount(uint8_t c); //设置响的次数1~N，表示间隔1s响几次
#endif //PWM_USE_TIM
#define FOREVER()		while(1)

/*-----------设备信息及参数配置定义------------*/
typedef struct {
	uint16_t type; //参数类型
	uint16_t idle; //IDLE_TIME，空闲时间
	uint16_t tx; //TX_TIMEOUT发送超时
	uint16_t heart; //HEART_BEART_TIMEOUT心跳超时
} Network_setting_t;
#define TYPE_MASK_IDLE_TIME						(1<<12)	//IDLE_TIME
#define TYPE_MASK_TX_TIMEOUT					(1<<8)	//TX_TIMEOUT
#define TYPE_MASK_HEART_TIMEOUT					(1<<4)	//HEART_BEAT_TIMEOUT
#define TYPE_MASK_SET_DEFAULT					(1<<0)	//还原成默认值
#define IS_VALID_IDLE_TIME(x)  					((x)>=20)	//IDLE_TIME >=20必须满足
#define IS_VALID_TX_TIMEOUT(x)					((x)>1000)//发送超时必须>1000
#define IS_VALID_HEART_TIMEOUT(heart,tx) 		((heart)>(tx)) 	//心跳必须>发送超时
//网络参数默认值
#define DEFAULT_VALUE_SET_TYPE						0x1110		//默认所有都有效
#define DEFAULT_VALUE_SET_IDLE						2000		//2000ms
#define DEFAULT_VALUE_SET_TX						2000		//2000ms
#define DEFAULT_VALUE_SET_HEART						5000		//5000ms
#if (USE_INTERNET||BOOT_LOADER)
typedef struct {
	uint8_t ip[4]; //以太网IP地址
	uint8_t ip_mask[4];//子网掩码
	uint8_t ip_gateway[4];//网关地址
	uint8_t ip_target[4];//目标IP地址
	uint16_t send_port;//目标端口
	uint16_t recv_port;//接收端口
}UDP_Net_Config_t;
#endif


//必须保证32位（4字节）对齐，因为flash写入是以32位为单位
typedef struct {
	uint32_t flag; //初始化标志	，0x0001--表示初始化过以下数据有效，0xffffffff--表示未初始化以下数据无效
	uint8_t mac[8]; //zigbee IEEE地址8字节，以太网MAC取前6个字节
	uint32_t sn; //产品SN序列号
	uint32_t ver_fw;	//固件版本号
	uint32_t ver_sw; 	//软件版本号，101表示1.0.1版本
	Network_setting_t sets; //参数设置
	uint32_t debug;		//是否开启调试模式
#if (USE_INTERNET||BOOT_LOADER)//只有RZG才有以太网
	UDP_Net_Config_t udp;//UDP参数配置
#endif //endof RZG
} Device_Info_t;

#define DEVICE_INFO_VALID		0x01					//设备信息数据有效
#define DEVICE_INFO_INVALID		(!DEVICE_INFO_VALID)	//设备信息数据无效
/*-----------SHELL用户交互------------*/
#define COM_KEY_RESET				"232start"
#define COM_KEY_SHELL				"232start"	//出厂设置串口密码，只有输入该密码才能通过串口恢复出厂值，大小写区分
#define COM_KEY_SHELL_DEBUGON		"-v"		//调试模式，打印所有调试信息
#define COM_KEY_SHELL_DEBUGOFF		"-n"		//关闭调试模式，正常启动
#define COM_KEY_SHELL_RESET 		"-r"		//恢复出厂设置
#define COM_KEY_SHELL_EXIT			"-q"		//退出SHELL
#define COM_KEY_SHELL_HELP			"-h"		//帮助
#define COM_KEY_SHELL_XUPDATE		"-u"		//使用Xmodem升级程序
#define COM_KEY_SHELL_ERASE			"-e"	//强制擦除应用程序
#define COM_KEY_SHELL_PRINTINFO		"-p"		//打印设备信息
#define COM_KEY_SHELL_SETINFO		"-s"		//设置设备信息

#define COM_KEY_SHELL_HELP_INFO 	"-v 调试模式\n\
-n 关闭调试模式\n\
-r 恢复出厂设置\n\
-q 退出DEBUG\n\
-u 使用Xmodem 1K升级程序\n\
-e 强制擦除应用程序\n\
-p 打印设备信息\n\
-s 设置设备信息\n\
-h 打印帮助信息\n"


extern uint8_t debugFlag; //调试开关

#define DPRINT(flag,format,args...)  do {if(flag) printf(format,##args);}while(0)
#define Dprintf(args...) DPRINT(debugFlag,##args)

/*-----------SHELL用户交互------------*/
#if RZB //以下为RZB独有
//RFID_51822读卡器接口 UART5 ,RX--PD2 ,TX--PC12
#define RFID_IF						UART5
#define RFID_IF_AF					GPIO_AF_UART5
#define RFID_IF_CLK_EN()			RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE)

#define RFID_IF_RX_PIN				GPIO_Pin_2	//PD2
#define RFID_IF_RX_PORT				GPIOD
#define RFID_IF_RX_CLK_EN()			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE)
#define RFID_IF_RX_CLK				RCC_AHB1Periph_GPIOD
#define RFID_IF_RX_SRC				GPIO_PinSource2

#define RFID_IF_TX_PIN				GPIO_Pin_12	//PC12
#define RFID_IF_TX_PORT				GPIOC
#define RFID_IF_TX_CLK_EN()			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE)
#define RFID_IF_TX_CLK				RCC_AHB1Periph_GPIOC
#define RFID_IF_TX_SRC				GPIO_PinSource12
#define RFID_IF_IRQ					USART5_IRQn



void Board_Rfid_Init();
uint16_t Board_Rfid_Nowait_Gets(uint8_t *key, uint16_t len);

#endif //endof if RZB

uint8_t Board_Is_RZB();
int aoti(char *str);
void Board_Version_Init();
void Board_Version_Print(uint32_t ver);
uint32_t Board_Version_Get();

#define BOARD_VERSION_MASK		((uint32_t)0x7FFF1F)	//年月日，从后往前1F--日（最大31），F-月（最大12），7FF-年（最大2046）

#endif //endof BOARD_H
