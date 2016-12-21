#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/**
 *@file board_rzg.c
 *@brief RZG板硬件接口定义(MCU-STM32F207ZG)
 *@author lxm
 *@date 2016.10.13
 *@version 0.1.0
 *@par Copyright (c):
 *      XXX公司
 *@par 修改日志
 *      0.1.0 xx 2016.10.11 创建文件
 **/

/*---------------------RZG硬件配置-------------------------
 *@details RZG硬件清单：
 *@details 1.UART4-作为调试串口(详见xprintf.h)
 *@details   TX  --PC10
 *@details   RX  --PC11
 *@details 2.SPI1-与MCR20A射频芯片通讯接口(详见spi.h MCR20A_port.h)
 *@details   SCK --PA5
 *@details   MOSI--PB5
 *@details   MISO--PA6
 *@details   CS  --PA4
 *@details 3.EXTI_Line9--MCR20A射频芯片中断接口(详见MCR20A_port.h)
 *@details   IRQ --PC9
 *@details   EXTI9_5_IRQn
 *@details   EXTI9_5_IRQHandler
 *@details 4.RST -MCR20A射频芯片复位引脚
 *@details   RST --PB15（低电平有效，高电平无效）
 *@details 5.LED-指示灯
 *@details   ALARM-PB7  告警灯
 *@details   RUN --PB8  系统运行指示灯
 *@details 6.GPIOA9--RS232_EN串口使能引脚
 *@details   RS232_EN--PA9
 *@details 7.ETH-以太网接口
 *@details   MDIO--PA2
 *@details   MDC--PC1
 *@details   TX_CLK--PC3
 *@details   TX_EN--PB11
 *@details   TXD_0--PB12
 *@details   TXD_1--PB13
 *@details   TXD_2--PC2
 *@details   TXD_3--PE2
 *@details   RX_CLK--PA1
 *@details   RX_DV--PA7
 *@details   CRS--PA0
 *@details   RX_ER--PB10
 *@details   COL--PA3
 *@details   RXD_0--PC4
 *@details   RXD_1--PC5
 *@details   RXD_2--PB0
 *@details   RXD_3--PB1
 *@details   RST_CPU--PB15
 *@details   RESET--PC6
 *@details   POWR_DOWN--PB14
 *@details
 *---------------------RZB硬件配置-------------------------
 *@details RZB硬件清单：
 *@details 1.UART4-作为调试串口(详见xprintf.h)
 *@details   TX  --PC10
 *@details   RX  --PC11
 *@details 2.SPI1-与MCR20A射频芯片通讯接口(详见spi.h MCR20A_port.h)
 *@details   SCK --PA5
 *@details   MOSI--PA7
 *@details   MISO--PA6
 *@details   CS  --PA1
 *@details 3.EXTI_Line1--MCR20A射频芯片中断接口(详见MCR20A_port.h)
 *@details   IRQ --PC9
 *@details   EXTI1_IRQn
 *@details   EXTI1_IRQHandler
 *@details 4.RST -MCR20A射频芯片复位引脚
 *@details   RST --PA2（低电平有效，高电平无效）
 *@details 5.LED-指示灯
 *@details   ALARM-PB7  告警灯
 *@details   RUN --PB8  系统运行指示灯
 *@details 6.SPI2--与nRF24L01通讯接口
 *@details   SCK --PB13
 *@details   MOSI--PB14
 *@details   MISO--PB15
 *@details   CSN  --PB12
 *@details 7.CE --nRF24L01使能控制引脚
 *@details 	 CE  --PA3
 *@details 8.GPIOA8--nRF24L01中断（不需要打开中断，只做普通输入IO）
 *@details   IRQ输入-PA8
 *@details 9.GPIOA9--RS232_EN串口使能引脚
 *@details   RS232_EN--PA9
 *@details 10.TIM2--蜂鸣器（PWM）定时器
 *@details    PWM输出--PE9
 *@details 11.UART5--nRF518822 RFID读卡器
 *@details    RX--- PD2
 *@details    TX--- PC12
 */

//#define RZG 			1	//0--RZG板，1--RZB板

//#define BOOT_LOADER		1 	//1--表示是启动程序，0--应用程序

#if BOOT_LOADER
#else
#if !RZG	//作为节点
/*---------RZB作为节点--------*/
#define RZB 			1
//节点标志，以下只能有且仅有一个为1
#define RFD_0 			1		//节点0
#define RFD_1 			0		//节点1
#define RFD_2 			0		//节点2
#define RFD_3 			0		//节点3
#else 	//作为网关
#define USE_INTERNET 	1
#define RZB 0
#endif //ENDOF !RZG

#endif //ENDOF BOOT_LOADER


/*---测试使用---*/
#define TEST 			1
/*----设备ZigBee/以太网模块物理地址（以太网地址取ZigBee地址的前6个字节）  ---*/

#define USE_AUTO_MAC 	1		//使用自动生成MAC地址

#if RZG
#define aExtendedAddress_B7 0x00
#define aExtendedAddress_B6 0x12
#define aExtendedAddress_B5 0x4B
#define aExtendedAddress_B4 0x00
#define aExtendedAddress_B3 0x00
#define aExtendedAddress_B2 0x01
#define aExtendedAddress_B1 0x21
#define aExtendedAddress_B0 0x00
#else
#if RFD_0	//RFD0
#define aExtendedAddress_B7 0x00
#define aExtendedAddress_B6 0x12
#define aExtendedAddress_B5 0x4B
#define aExtendedAddress_B4 0x00
#define aExtendedAddress_B3 0x00
#define aExtendedAddress_B2 0x01
#define aExtendedAddress_B1 0x21
#define aExtendedAddress_B0 0x01
#elif RFD_1 	//RFD1
#define aExtendedAddress_B7 0x00
#define aExtendedAddress_B6 0x12
#define aExtendedAddress_B5 0x4B
#define aExtendedAddress_B4 0x00
#define aExtendedAddress_B3 0x00
#define aExtendedAddress_B2 0x01
#define aExtendedAddress_B1 0x21
#define aExtendedAddress_B0 0x02
#elif RFD_2
#define aExtendedAddress_B7 0x00
#define aExtendedAddress_B6 0x12
#define aExtendedAddress_B5 0x4B
#define aExtendedAddress_B4 0x00
#define aExtendedAddress_B3 0x00
#define aExtendedAddress_B2 0x01
#define aExtendedAddress_B1 0x21
#define aExtendedAddress_B0 0x03
#else //RFD3
#define aExtendedAddress_B7 0x00
#define aExtendedAddress_B6 0x12
#define aExtendedAddress_B5 0x4B
#define aExtendedAddress_B4 0x00
#define aExtendedAddress_B3 0x00
#define aExtendedAddress_B2 0x01
#define aExtendedAddress_B1 0x21
#define aExtendedAddress_B0 0x67
#endif

#endif //if RZG

/*------数据加密开关-------*/
#define DATA_ENCRYPTION_ENABLE     0 //1--使能数据加密，0--禁用数据加密
/*-------以太网配置---*/

#define BOARD_IP_0				192
#define BOARD_IP_1 				168
#define BOARD_IP_2				10
#define BOARD_IP_3				111

#define BOARD_NETMASK_0			255
#define BOARD_NETMASK_1			255
#define BOARD_NETMASK_2			254
#define BOARD_NETMASK_3			0

#define BOARD_GW_0				192
#define BOARD_GW_1				168
#define BOARD_GW_2				10
#define BOARD_GW_3				1

#define BOARD_IP_SERVER_0		192
#define BOARD_IP_SERVER_1		168
#define BOARD_IP_SERVER_2		10
#define BOARD_IP_SERVER_3		10

#define UDP_SEND_PORT     		1031
#define UDP_RECV_PORT			1030

/*--------软件版本----------*/

#define DEVICE_SN				0x00000001	//设备序列号
#if BOOT_LOADER
#define VER_FW_DF				201			//固件版本号,
#define VER_SW_DF				000			//软件版本号，默认值

#define BOOT_MD5_CHECK			1			//开启MD5加密校验
#else
#define VER_FW_DF				000			//固件版本号,默认值
#define VER_SW_DF				212			//软件版本号
#endif

#define DEBUG_PRINT				1			//调试打印

#define DEBUG_SHELL				0			//调试SHELL打开开关，1--开启Shell功能，0--关闭Shell功能

#endif//  BOARD_CONFIG_H
