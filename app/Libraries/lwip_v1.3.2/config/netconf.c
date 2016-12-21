/**
 ******************************************************************************
 * @file    netconf.c
 * @author  MCD Application Team
 * @version V1.1.0
 * @date    07-October-2011
 * @brief   Network connection configuration
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "board.h"
#include "netconf.h"
#include <stdio.h>

#ifdef USE_INTERNET 	//使用以太网才需要以下函数


/* Private typedef -----------------------------------------------------------*/
#define MAX_DHCP_TRIES        4

/* Private define ------------------------------------------------------------*/
typedef enum {
	DHCP_START = 0, DHCP_WAIT_ADDRESS, DHCP_ADDRESS_ASSIGNED, DHCP_TIMEOUT
} DHCP_State_TypeDef;
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif netif;
uint32_t TCPTimer = 0;
uint32_t ARPTimer = 0;
uint32_t IPaddress = 0;

#ifdef USE_DHCP
uint32_t DHCPfineTimer = 0;
uint32_t DHCPcoarseTimer = 0;
DHCP_State_TypeDef DHCP_state = DHCP_START;
#endif

/* Private functions ---------------------------------------------------------*/
void LwIP_DHCP_Process_Handle(void);

//从设备信息中读取IP地址
void IP4_Addr_Set(struct ip_addr *ipaddr, struct ip_addr *netmask,
		struct ip_addr *gw) {
	Device_Info_t *ptr = Board_DeviceInfo_Get();
	if (ptr->flag == DEVICE_INFO_VALID) {

		IP4_ADDR(ipaddr, ptr->udp.ip[0], ptr->udp.ip[1], ptr->udp.ip[2], ptr->udp.ip[3]);
		IP4_ADDR(netmask, ptr->udp.ip_mask[0], ptr->udp.ip_mask[1], ptr->udp.ip_mask[2],
				ptr->udp.ip_mask[3]);
		IP4_ADDR(gw, ptr->udp.ip_gateway[0], ptr->udp.ip_gateway[1], ptr->udp.ip_gateway[2],
				ptr->udp.ip_gateway[3]);
	} else {
		//地址无效，Flash未初始化，采用默认参数
		IP4_ADDR(ipaddr, BOARD_IP_0, BOARD_IP_1, BOARD_IP_2, BOARD_IP_3);
		IP4_ADDR(netmask, BOARD_NETMASK_0, BOARD_NETMASK_1, BOARD_NETMASK_2,
				BOARD_NETMASK_3);
		IP4_ADDR(gw, BOARD_GW_0, BOARD_GW_1, BOARD_GW_2, BOARD_GW_3);
	}
}

/**
 * @brief  Initializes the lwIP stack
 * @param  None
 * @retval None
 */
void LwIP_Init(void) {
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
#ifndef USE_DHCP
	uint8_t iptab[4];
	uint8_t iptxt[20];
#endif

	/* Initializes the dynamic memory heap defined by MEM_SIZE.*/
	mem_init();

	/* Initializes the memory pools defined by MEMP_NUM_x.*/
	memp_init();

#ifdef USE_DHCP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else
	IP4_Addr_Set(&ipaddr, &netmask, &gw);

#if PRINT_IP
	printf("\n\r Static IP address : %d.%d.%d.%d\r\n", BOARD_IP);
#endif	//endif print_ip
#endif

	/* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
	 struct ip_addr *netmask, struct ip_addr *gw,
	 void *state, err_t (* init)(struct netif *netif),
	 err_t (* input)(struct pbuf *p, struct netif *netif))

	 Adds your network interface to the netif_list. Allocate a struct
	 netif and pass a pointer to this structure as the first argument.
	 Give pointers to cleared ip_addr structures when using DHCP,
	 or fill them with sane numbers otherwise. The state pointer may be NULL.

	 The init function pointer must point to a initialization function for
	 your ethernet netif interface. The following code illustrates it's use.*/
	netif_add(&netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init,
			&ethernet_input);

	/*  Registers the default network interface.*/
	netif_set_default(&netif);

	/*  When the netif is fully configured this function must be called.*/
	netif_set_up(&netif);
}

/**
 * @brief  Called when a frame is received
 * @param  None
 * @retval None
 */
void LwIP_Pkt_Handle(void) {
	/* Read a received packet from the Ethernet buffers and send it to the lwIP for handling */
	ethernetif_input(&netif);
}

/**
 * @brief  LwIP periodic tasks
 * @param  localtime the current LocalTime value
 * @retval None
 */
void LwIP_Periodic_Handle(__IO uint32_t localtime) {
#if LWIP_TCP
	/* TCP periodic process every 250 ms */
	if (localtime - TCPTimer >= TCP_TMR_INTERVAL) {
		TCPTimer = localtime;
		tcp_tmr();
	}
#endif

	/* ARP periodic process every 5s */
	if ((localtime - ARPTimer) >= ARP_TMR_INTERVAL) {
		ARPTimer = localtime;
		etharp_tmr();
	}

#ifdef USE_DHCP
	/* Fine DHCP periodic process every 500ms */
	if (localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
	{
		DHCPfineTimer = localtime;
		dhcp_fine_tmr();
		if ((DHCP_state != DHCP_ADDRESS_ASSIGNED)&&(DHCP_state != DHCP_TIMEOUT))
		{
			/* process DHCP state machine */
			LwIP_DHCP_Process_Handle();
		}
	}

	/* DHCP Coarse periodic process every 60s */
	if (localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
	{
		DHCPcoarseTimer = localtime;
		dhcp_coarse_tmr();
	}

#endif
}

#ifdef USE_DHCP
/**
 * @brief  LwIP_DHCP_Process_Handle
 * @param  None
 * @retval None
 */
void LwIP_DHCP_Process_Handle()
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	uint8_t iptab[4];
	uint8_t iptxt[20];

	switch (DHCP_state)
	{
		case DHCP_START:
		{
			dhcp_start(&netif);
			IPaddress = 0;
			DHCP_state = DHCP_WAIT_ADDRESS;
#ifdef USE_LCD
			LCD_DisplayStringLine(Line4, (uint8_t*)"     Looking for    ");
			LCD_DisplayStringLine(Line5, (uint8_t*)"     DHCP server    ");
			LCD_DisplayStringLine(Line6, (uint8_t*)"     please wait... ");
#endif
		}
		break;

		case DHCP_WAIT_ADDRESS:
		{
			/* Read the new IP address */
			IPaddress = netif.ip_addr.addr;

			if (IPaddress!=0)
			{
				DHCP_state = DHCP_ADDRESS_ASSIGNED;

				/* Stop DHCP */
				dhcp_stop(&netif);

#ifdef USE_LCD
				iptab[0] = (uint8_t)(IPaddress >> 24);
				iptab[1] = (uint8_t)(IPaddress >> 16);
				iptab[2] = (uint8_t)(IPaddress >> 8);
				iptab[3] = (uint8_t)(IPaddress);

				sprintf((char*)iptxt, " %d.%d.%d.%d", iptab[3], iptab[2], iptab[1], iptab[0]);

				LCD_ClearLine(Line4);
				LCD_ClearLine(Line5);
				LCD_ClearLine(Line6);

				/* Display the IP address */
				LCD_DisplayStringLine(Line7, (uint8_t*)"IP address assigned ");
				LCD_DisplayStringLine(Line8, (uint8_t*)"  by a DHCP server  ");
				LCD_DisplayStringLine(Line9, iptxt);
#endif

			}
			else
			{
				/* DHCP timeout */
				if (netif.dhcp->tries > MAX_DHCP_TRIES)
				{
					DHCP_state = DHCP_TIMEOUT;

					/* Stop DHCP */
					dhcp_stop(&netif);

					/* Static address used */

					My_IP4_ADDR(&ipaddr, BOARD_IP);
					My_IP4_ADDR(&netmask, BOARD_NETMASK);
					My_IP4_ADDR(&gw, BOARD_WG);
					netif_set_addr(&netif, &ipaddr , &netmask, &gw);

#ifdef USE_LCD
					LCD_DisplayStringLine(Line7, (uint8_t*)"    DHCP timeout    ");

					iptab[0] = IP_ADDR3;
					iptab[1] = IP_ADDR2;
					iptab[2] = IP_ADDR1;
					iptab[3] = IP_ADDR0;

					sprintf((char*)iptxt, "  %d.%d.%d.%d", iptab[3], iptab[2], iptab[1], iptab[0]);

					LCD_ClearLine(Line4);
					LCD_ClearLine(Line5);
					LCD_ClearLine(Line6);

					LCD_DisplayStringLine(Line8, (uint8_t*)"  Static IP address   ");
					LCD_DisplayStringLine(Line9, iptxt);
#endif

				}
			}
		}
		break;
		default: break;
	}
}
#endif      




#endif	 //ENDOF RZG
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
