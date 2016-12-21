#ifndef UDP_SERVER_H
#define UDP_SERVER_H
#include "board.h"

#if USE_INTERNET //只有使用网络才需要
#include "netconf.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "xprintf.h"

//udp通讯的PCB
static struct udp_pcb *udpPcbPrivate;
static struct udp_pcb *udprecv;
extern uint8_t zg_get_settings;

static struct ip_addr ipAddrTarget; //UDP 目标地址
static uint16_t sendPort; //UDP 发送端口号
static uint16_t recvPort; //UDP 接收端口号

struct ip_addr * UDP_Target_Get() {
	return &ipAddrTarget;
}
//设置UDP目标IP地址
void UDP_Target_Init() {
	Device_Info_t *devInfo = Board_DeviceInfo_Get();
	IP4_ADDR(&ipAddrTarget, devInfo->udp.ip_target[0],
			devInfo->udp.ip_target[1], devInfo->udp.ip_target[2],
			devInfo->udp.ip_target[3]);

	sendPort = devInfo->udp.send_port;

	recvPort = devInfo->udp.recv_port;

}

//向目标发送数据，自己封装的udp_send函数，应用方便UDP服务器发送数据
uint8_t UDP_Target_Send(struct udp_pcb *pcb, struct pbuf *p) {
	return udp_sendto(pcb, p, &ipAddrTarget, sendPort);
}

//size 大小（字节数）
void swap_buf(uint16_t *out, uint16_t *in, uint16_t size) {
	for (uint16_t i = 0; i < (size / sizeof(uint16_t)); i++) {
		out[i] =ntohs( in[i]);
	}
}

///***********************************************************************
//函数名称：udp_data_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,struct ip_addr *addr, u16_t port)
//功    能：udp数据接收和发送
//输入参数：
//输出参数：
//编写时间：2013.4.25
//编 写 人：
//注    意：这是一个回调函数，当一个udp段到达这个连接时会被调用
//***********************************************************************/
udp_data_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
		struct ip_addr *addr, uint16_t port) {
	struct ip_addr destAddr = *addr; /* 获取远程主机 IP地址 */
	if (p != NULL) {

		//存储数据
		if (p->len == sizeof(Network_setting_t)) {
			Network_setting_t settings;
			//网络参数采用的都是uint16_t定义的，因此直接swap_buf
			swap_buf(&settings, p->payload, p->len);

			//设置网络参数
			if (Board_Network_Set(&settings)) { //参数设置成功，将标志位置为1，以备发送给节点
				zg_get_settings = 1;
			}
		}

		if (p->len == sizeof(UDP_Net_Config_t)) { //接收到ZG UDP参数配置数据
			Device_Info_t info_org;

			//printf("接收到UDP设置信息，ZG将重启！\n");
			Board_DeviceInfo_Read(&info_org); //获取板上现有参数

			//网络字节序标准为大端在前，IP[0~3]采用的就是大端在前的读取方式
			//udp参数中只有最后4个字节为uint16，前面的全都是uint8_t不需要大小端转换
			memcpy(&(info_org.udp),p->payload,p->len-4);
			uint16_t *pdata=(uint16_t *)(p->payload+(p->len-4));
			info_org.udp.send_port=ntohs(*pdata);
			pdata++;	//后移
			info_org.udp.recv_port=ntohs(*pdata);

			Board_DeviceInfo_Write(&info_org);

			Board_Soft_Reset();
		}

		/******将数据原样返回*******************/
		udp_sendto(pcb, p, &destAddr, port); /* 将收到的数据再发送出去 */
		pbuf_free(p); /* 释放该UDP段 */
	}

}

/***********************************************************************
 函数名称：UDP_server_init(void)
 功    能：完成UDP服务器的初始化，主要是使得UDP通讯快进入监听状态
 输入参数：
 输出参数：
 编写时间：2013.4.25
 编 写 人：
 注    意：
 ***********************************************************************/
void UDP_Server_Init() {
	//初始化目标参数
	UDP_Target_Init();
	udpPcbPrivate = udp_new(); //申请udp控制块
	udp_bind(udpPcbPrivate, IP_ADDR_ANY, recvPort); /* 绑定接收数据端口号（作为udp服务器） */
	udp_recv(udpPcbPrivate, udp_data_recv, NULL); /* 设置UDP段到时的回调函数 */
	//发送数据需要重新制定端口
}
void UDP_Client_Init() {
	udpPcbPrivate = udp_new(); //申请udp控制块
	udp_bind(udpPcbPrivate, IP_ADDR_ANY, 1010); /* 绑定本地IP地址和端口号（作为up服务器） */
	struct ip_addr ipaddr;
	Device_Info_t *ptr = Board_DeviceInfo_Get();
	IP4_ADDR(&ipaddr, ptr->udp.ip_target[0], ptr->udp.ip_target[1],
			ptr->udp.ip_target[2], ptr->udp.ip_target[3]);
	udp_connect(udpPcbPrivate, &ipaddr, ptr->udp.send_port); //连接服务器
	udp_recv(udpPcbPrivate, udp_data_recv, NULL);

}

struct udp_pcb * UDP_Client_Pcb_Get() {
	return udpPcbPrivate;
}

#endif //ENDOF RZG
#endif  //UDP_SERVER_H
