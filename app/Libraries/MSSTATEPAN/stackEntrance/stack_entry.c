/**
 * 数据流向(单向传输)：RZB（节点）->RZG（协调器/UDP客户端）->BOX（UDP服务器）
 * Zigbee通讯：ZigBee面向连接，应用层有重发（包括数据包重发和心跳包重发）
 * UDP通讯：面向无连接，应用层不重发
 * 心跳机制：心跳只在无数据发送时产生，有数据通讯时不发送心跳；
 */

#include "msstate_lrwpan.h"
#include "stack_entry.h"
#include "board.h"
#ifdef USE_INTERNET
#include "netconf.h"
#include "lwip/udp.h"
#endif

/*-----------------------------全局变量----------------------*/
#define ADDR_ALL_DEVICES		0xFFFF		//广播给所有设备（包括节点、路由器和协调器）
#define ADDR_ALL_ENDNOTES		0xFFFD		//广播给所有节点（必须要节点加入到网络才发送）
#define ADDR_ALL_ROUTERS		0xFFFC		//广播给所有路由器
LADDR_UNION dstADDR = { 0 }; //默认为协调器
UINT32 my_timer;
UINT32 last_tx_start;
UINT8 app_started; //应用程序启动标志，0--未启动，1--已经启动
UINT32 app_startTime; //超时500ms启动
UINT32 heart_startTime; //心跳包开始时间

#define ZIGBEE_DATA_STATIC_LEN		8//Zigbee_packed_t中前8个字节为固定长度
#if DATA_ENCRYPTION_ENABLE
#define HEART_BEAT_LEN				16//开启加密后，数据最小长度应该为16，因此心跳设置为16
#else
#define HEART_BEAT_LEN			ZIGBEE_DATA_STATIC_LEN	//心跳包长度，最小数据长度
#endif

typedef struct {
	UINT8 id[4]; //标签ID号
	UINT8 move[2]; //移动
	UINT8 rssi; //RSSI信号强度
	UINT8 power; //电量信息
} Rfid_Data_t; //RFID数据，8个字节
#define RFID_PACK_COUNT_MAX		9	//最大9个，对应的Zigbee_packed_t长度为80
#define RFID_PACK_COUNT			9	//实际PACK包数，不能大于最大数RFID_PACK_COUNT_MAX
typedef struct {
	UINT8 len; //数据长度
	UINT8 type; //是否是心跳包,高4位为心跳,1--心跳包，0--数据包
	UINT8 id[3]; //zigbee设备ID号
	UINT8 unUsed[3]; //备用，填充满足8字节对齐
	Rfid_Data_t rfid[RFID_PACK_COUNT]; //节点数据包，最大容纳9个节点数据
} Zigbee_packed_t;
static Zigbee_packed_t data_packed; //组包后的数据包
static UINT8 data_count = 0; //组包计数器,

#if DATA_ENCRYPTION_ENABLE	//开启数据加密
//加密数据常量
const UINT8 AES_KEY[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
	0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
const UINT8 AES_CNT[16] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
	0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
#endif

#define TX_RETRY_COUNT 			3

//#define TX_TIMEOUT_MS  		2000 //发送失败超时，单位ms
//#define IDLE_TIMEOUT_MS 		2000	//发送成功的间隔时间，单位ms，最小设置为20ms，小于20无效
//#define HEART_BEAT_TIMEOUT  	5000	//5000ms,心跳包超时时间5s
Network_setting_t zigbee_sets; //网络参数，网络初始化时设置

//获取到了设置数据，则广播发送给所有节点0xFFFD
UINT8 zg_get_settings; //ZG UDP是否获取到了网络设置数据，0-无设置数据，1--有设置数据

//节点获取到了设置参数，需要重启发送状态（发送状态机复位）
UINT8 zigbee_get_settings; //节点（zigbee）接收到了参数设置，0-未接收到，1--接收到了需要处理
/*-----------------------------函数声明----------------------*/
void Zigbee_Data_Service(void);
void printJoinInfo(void);

/*-----------------------------函数定义----------------------*/

void Network_Config() {
	Network_setting_t *sets; //设置指针
	Device_Info_t *ptr = Board_DeviceInfo_Get();
	sets = &(ptr->sets);

	if ((sets->type) & (TYPE_MASK_IDLE_TIME)) { //Flash中的数据有效
		zigbee_sets.idle = sets->idle;
		zigbee_sets.type = sets->type; //保存设置类型，只要有一个参数有效就保存
	} else { //Flash中数据无效
		zigbee_sets.idle = DEFAULT_VALUE_SET_IDLE; //采用缺省值
	}

	if ((sets->type) & (TYPE_MASK_TX_TIMEOUT)) { //Flash中的数据有效
		zigbee_sets.tx = sets->tx;
		zigbee_sets.type = sets->type; //保存设置类型，只要有一个参数有效就保存
	} else { //Flash中数据无效
		zigbee_sets.tx = DEFAULT_VALUE_SET_TX; //采用缺省值
	}

	if ((sets->type) & (TYPE_MASK_HEART_TIMEOUT)) { //Flash中的数据有效
		zigbee_sets.heart = sets->heart;
		zigbee_sets.type = sets->type; //保存设置类型，只要有一个参数有效就保存
	} else { //Flash中数据无效
		zigbee_sets.heart = DEFAULT_VALUE_SET_HEART; //采用缺省值
	}

}

//初始化ZigBee_Data相关变量值，每次发送成功都需要 调用一次，确保下次发送的数据正常
void Zigbee_Data_Init() {
	static UINT8 _once = 1;
	//清楚数据计数器
	data_count = 0;

	//清楚数据内容
	data_packed.len = 0;
	data_packed.type = 0; //心跳位为0，表示数据
#if !DATA_ENCRYPTION_ENABLE	//如果加密开启 的话，每次都需要重新初始化data_packed，否则只需要执行一次
	if (_once)
#endif
	{
		UINT8 *mac = Board_Mac_Get_Ptr();
		//取MAC地址的前6个字节作为ID
		for (int i = 0; i < sizeof(data_packed.id); i++) {
			data_packed.id[i] = mac[i];
		}

		_once = 0;
	}
}


#ifdef LRWPAN_COORDINATOR//RZG作为协调器
//返回0--表示成功，非0--表示失败
UINT8 Zg_Data_Send() {
	BYTE len, *ptr;
	UINT8 ret;

	len = aplGetRxMsgLen();
	ptr = aplGetRxMsgData();

	//以太网发送数据
	struct udp_pcb *pcb = UDP_Client_Pcb_Get();
	struct pbuf *udpbuf;
#if DATA_ENCRYPTION_ENABLE	//开启数据加密
	if(len>=16) { //必须保证数据长度大于等于16个字节
		//只加密前16个字节
		halDataEncryptAndDecrypt(AES_KEY, ptr, AES_CNT, ptr);
	}
#endif
	udpbuf = pbuf_alloc(PBUF_RAW, len, PBUF_REF);
	if (NULL == udpbuf)
	return -1; //ERROR
	udpbuf->payload = ptr;

	ret = UDP_Target_Send(pcb, udpbuf);
	//udp_sendto(pcb, udpbuf,&ipAddrTarget,devInfo->udp.port);

	pbuf_free(udpbuf);

	Dprintf("Got len %d form NetId 0x%x\n", len,aplGetRxSrcSADDR());
	return ret;
}

void Zg_Heart_Send() {

	data_packed.len = HEART_BEAT_LEN; //心跳包长度
	data_packed.type = (1 << 4)|(1);//高4位为心跳标志，心跳位为1；低4位为ZG标志

	//以太网发送数据
	struct udp_pcb *pcb = UDP_Client_Pcb_Get();
	struct pbuf *udpbuf;
	udpbuf = pbuf_alloc(PBUF_RAW, data_packed.len, PBUF_REF);
	if (NULL != udpbuf) {
#if DATA_ENCRYPTION_ENABLE	//开启数据加密
		if (sizeof(data_packed) >= 16) {
			//只加密前16字节，加密完后复制回原先数据空间
			halDataEncryptAndDecrypt(AES_KEY, &data_packed, AES_CNT,
					&data_packed);
		}
#endif
		udpbuf->payload = &data_packed;

		UDP_Target_Send(pcb, udpbuf);
		pbuf_free(udpbuf);

		Dprintf("ZG Send HeartBeat.\n");
	} else {
		//内存申请失败，异常
	}
}

#else//RZB作为节点
//设置节点网络参数
void Zigbee_Recv_Settings() {
	BYTE len, *ptr;

	len = aplGetRxMsgLen();
	ptr = aplGetRxMsgData();
	if (len == sizeof(Network_setting_t)) { //获取到了配置参数数据
		Network_setting_t *settings = ptr;

		//设置网络参数
		Board_Network_Set(settings); //实际测试需要9183us，耗时很长！

		zigbee_get_settings = 1; //置节点接收到参数设置标志为1
	}
}

//Zigbee网络心跳
UINT8 Zigbee_Heart_Beat() {
	UINT8 ret = 0; //返回值，0--未发送，>=1--已发送
	UINT8 *mac;
	//Zigbee_data_t heartbeat;
	data_packed.len = HEART_BEAT_LEN; //心跳包长度
	data_packed.type = (1 << 4); //心跳包

	if (halMACTimerNowDelta(heart_startTime) > zigbee_sets.heart) {

#if DATA_ENCRYPTION_ENABLE	//开启数据加密
		if (sizeof(data_packed) >= 16) {
			//只加密前16字节，加密完后复制回原先数据空间
			halDataEncryptAndDecrypt(AES_KEY, &data_packed, AES_CNT,
					&data_packed);
		}
#endif

		//心跳计时时间到，发送一帧心跳包
		aplSendMSG(APS_DSTMODE_SHORT, &dstADDR, 1, //dst EP
				0, //cluster is ignored for direct message
				1, //src EP
				&data_packed, HEART_BEAT_LEN, //msg length
				apsGenTSN(), FALSE);
		ret = 1;
		Dprintf("No Data,Sending HeartBeat.\n");
	}
	return ret;
}

//数据发送
#define RFID_GETDATA 1

UINT8 Zigbee_Data_Send() {
	UINT8 ret = 0; //返回值，0--未发送，>=1--已发送
	UINT8 len;
	//保存数据长度
	len = data_packed.len;
	//有数据发送则开始发送
	if (len) { //数据长度>0标识有数据
#if DATA_ENCRYPTION_ENABLE	//开启数据加密
	if (sizeof(data_packed) >= 16) {
		//只加密前16字节，加密完后复制回原先数据空间
		halDataEncryptAndDecrypt(AES_KEY, &data_packed, AES_CNT,
				&data_packed);
	}
#endif

		aplSendMSG(APS_DSTMODE_SHORT, &dstADDR, 1, //dst EP
				0, //cluster is ignored for direct message
				1, //src EP
				&data_packed, len, //msg length
				apsGenTSN(), FALSE);

		Dprintf("Send Data len=%d\n", len);
		ret = 1;
	} else { //有数据则不发送心跳
		ret = Zigbee_Heart_Beat();
	}
	return ret;
}
static int rfid_count=0;
//ZigBee网络空闲状态，检测RFID数据并进行RFID组包
UINT8 Zigbee_Idle_Service() {
	UINT8 ret = 0;
	//RFID一个数据包，采用非阻塞方式读取数据（一个周期可能只读到了一半的数据，需要静态保存）
	static Rfid_Data_t rfid;
#if !RFID_GETDATA
	//真实情况
	UINT16 bytes;
	//非阻塞方式读取数据，接收缓冲必须是静态的
	bytes=Board_Rfid_Nowait_Gets(&rfid,sizeof(rfid));

	if(bytes==sizeof(rfid))
#else
	if (RFID_GETDATA)
#endif
	{
		rfid.id[0]=rfid_count++;
		//如果data_count等于RFID_PACK_COUNT，则表示上次打包好的数据还没发送，这次不处理
		if (data_count < RFID_PACK_COUNT) {
			//数据长度为：包前5个字节固定长度+RFID数据包个数*一个RFID数据包长度
			data_packed.len = ZIGBEE_DATA_STATIC_LEN
					+ sizeof(Rfid_Data_t) * (data_count + 1);

			//拷贝数据
			halUtilMemCopy(&(data_packed.rfid[data_count]), &rfid,
					sizeof(Rfid_Data_t));

			data_count++; //计数器增加

			//RFID数据包数达到了最大数，RFID_PACK_COUNT=9，则一个包组完了
			if (data_count == RFID_PACK_COUNT) {
				data_count = 0;
				ret = 1; //组包达到了最大值，需要立即发送
			}
		}
	}
	return ret;
}

#endif//endof LRWPAN_COORDINATOR
//协议栈维护
void Zigbee_Data_Service() {
	typedef enum {
		ENUM_TX_START = 0,
		ENUM_TX_WAIT,
		ENUM_RX_START,
		ENUM_RX_WAIT,
		ENUM_RADIO_IDLE
	} State_t;
	State_t state = ENUM_TX_START;
	uint32_t idle_time; //记录开始进入空闲状态的时间
	uint32_t tx_time;
	uint8_t tx_retry; //发送重试计数

	heart_startTime = 0; //初始化心跳计时为0

	tx_retry = TX_RETRY_COUNT;

	//初始化数据
	Zigbee_Data_Init();


	while (1) {
		apsFSM(); //wait for finish

#ifdef LRWPAN_COORDINATOR
		//协调器维护以太网
		//LwIP_Periodic_Handle(GetTimeMS());//处理接收或dhcp
		//以太网不重发数据
		if ((halMACTimerNowDelta(heart_startTime)) > zigbee_sets.heart) {
			Zg_Heart_Send(); //发送UDP网络心跳
			heart_startTime = halGetMACTimer();
		}
		if (zg_get_settings) { //参数设置数据需要发送
			zg_get_settings = 0;//清楚标志
			dstADDR.saddr = ADDR_ALL_DEVICES;//指向地址为所有节点，广播发送

			aplSendMSG(APS_DSTMODE_SHORT, &dstADDR, 1,//dst EP
					0,//无效
					1,//src EP
					&zigbee_sets, sizeof(zigbee_sets),//将设置参发送
					apsGenTSN(), FALSE);
		}

#else

		//节点设置参数，强制退出上次发送，重新进入TX_START状态
		if (zigbee_get_settings) {
			state = ENUM_TX_START; //强制重启状态机，重新进入初始状态
			zigbee_get_settings = 0; //清楚标志
		}

		switch (state) {
		case ENUM_TX_START: //发送状态
			//添加发送数据
			if (Zigbee_Data_Send() == 0) { //未产生任何数据发送，重新进入发送状态
				//state = ENUM_TX_START; //重新进入发送状态
				break;//以下都不处理
			}

			tx_time = halGetMACTimer();
			state = ENUM_TX_WAIT;
			break;
		case ENUM_TX_WAIT:
			if (apsBusy()) {
				break; //发送还没完成，直接退出
			}
			if (aplGetStatus() == LRWPAN_STATUS_SUCCESS) {
				//发送成功
				state = ENUM_RADIO_IDLE; //进入空闲状态，等待一段时间后重新发送
				//空闲计时开始
				idle_time = halGetMACTimer();

				//重置心跳包计时，不管是数据还是心跳发送，只要成功即重置心跳包发送时间
				heart_startTime = halGetMACTimer();

				tx_retry = TX_RETRY_COUNT;

				Zigbee_Data_Init(); //发送成功，还原数据为初始值
			} else { //发送失败，进入超时判定
				if ((halMACTimerNowDelta(tx_time)) > zigbee_sets.tx) {
					if (tx_retry > 0) {
						tx_retry--;
						Dprintf("Send Retry.\n");
						state = ENUM_TX_START; //重新进入发送状态
					}
					if (0 == tx_retry) { //重发次数用尽
						//蜂鸣器报警，默认3声
						if (Board_PWM_IsOff()) {
							Board_PWM_SetCount(3);
							Board_PWM_On();
						}
						return; //网络异常，退出本函数，进入重新搜索网络状态
					}

				}

			}

			break;
		case ENUM_RADIO_IDLE:
			if ((halMACTimerNowDelta(idle_time)) > zigbee_sets.idle) { //2s空闲超时
				state = ENUM_TX_START;
			} else {
				//空闲状态，进行RFID数据组包

				if (Zigbee_Idle_Service()) { //有数据包需要发送，则退出空闲状态，进入发送状态
#if RFID_GETDATA	//调试使用，真实环境不用
					if ((halMACTimerNowDelta(idle_time))
							> (zigbee_sets.idle / 10)) //大于1/10则立即发送当前组完的数据包
#endif
						state = ENUM_TX_START;
				}
			}
			break;
		default:
			break;
		}

#endif //endof #ifdef LRWPAN_COORDINATOR
	}
}

void printJoinInfo(void) {

	Dprintf(
			"本机短地址: 0x%X，父节点短地址：0x%X\n", aplGetMyShortAddress(), aplGetParentShortAddress());
	if (debugFlag) {
		BYTE *ptr = aplGetParentLongAddress();
		Dprintf("父节点长地址： ");
		for (uint8_t i = 0; i < 8; i++) {
			Dprintf("%02X", ptr[i]);
		}
		Dprintf("\n");
	}
}

typedef enum _JOIN_STATE_ENUM {
	JOIN_STATE_START_JOIN,
	JOIN_STATE_WAIT_FOR_JOIN,
	JOIN_STATE_RUN_APP1,
	JOIN_STATE_RUN_APP2,
	JOIN_STATE_RUN_APP3,
	JOIN_STATE_START_REJOIN,
	JOIN_STATE_WAIT_FOR_REJOIN,
} JOIN_STATE_ENUM;

JOIN_STATE_ENUM joinState;

#define MAX_REJOIN_FAILURES 3 //最大重连次数
//带重连机制
void stack_main(void) {

	UINT8 failures;

	my_timer = 0;
	halInit(); //初始化射频硬件
	aplInit(); //init the stack

	//conPrintConfig();

	Network_Config(); //配置网络参数

	EVB_LED1_OFF();
	EVB_LED2_OFF();

	joinState = JOIN_STATE_START_JOIN; //main while(1) initial state

	//debug_level = 10;

	while (1) {
		apsFSM();

		switch (joinState) {

		case JOIN_STATE_START_JOIN:
			EVB_LED1_OFF();
#ifdef LRWPAN_COORDINATOR
			aplFormNetwork()
			;
#else
			aplJoinNetwork();
#endif
			joinState = JOIN_STATE_WAIT_FOR_JOIN;
			break;
		case JOIN_STATE_WAIT_FOR_JOIN:
			if (apsBusy())
				break; //if stack is busy, continue
#ifdef LRWPAN_COORDINATOR
				if (aplGetStatus() == LRWPAN_STATUS_SUCCESS) {
					printf("网络建立完成。\n");
					EVB_LED1_ON(); //turn on LED1 indicating network is formed.
					joinState = JOIN_STATE_RUN_APP1;
				} else {
					//only reason for this to fail is some sort of hardware failure
					printf("网络建立失败，等待2s之后重试！\n");
					delayMS(2000);
					joinState = JOIN_STATE_START_JOIN;
				}
#else
			if (aplGetStatus() == LRWPAN_STATUS_SUCCESS) {
				EVB_LED1_ON();
				printf("加入网络成功！\n");
				printJoinInfo();
				joinState = JOIN_STATE_RUN_APP1;
			} else {
				printf("加入网络失败！等待10s之后重试！\n");
				//蜂鸣器报警,2声
				if (Board_PWM_IsOff()) {
					Board_PWM_SetCount(2);
					Board_PWM_On();
				}
				delayMS(10 * 1000);
				joinState = JOIN_STATE_START_JOIN;
			}
#endif

			break;

		case JOIN_STATE_RUN_APP1:
#ifdef LRWPAN_COORDINATOR

			Zigbee_Data_Service(); //用户程序入口

#else

			Dprintf("开始启动应用程序!\n");
			joinState = JOIN_STATE_RUN_APP2;
			if (app_started == 0) {
				//启动app计时，超过500ms则自动开始执行UserApp
				app_startTime = halGetMACTimer();
				app_started = 1;
			}
#endif
			break;
		case JOIN_STATE_RUN_APP2: //延迟一段时间运行用于程序，等待网络稳定
			//添加发送触发数据发送的条件，这里是等待200ms
			if ((halMACTimerNowDelta(app_startTime)) < 200) {
				break; //时间未到则退出重新等待
			} else {
				//没有break;所以可以直接进入下一个状态
				app_started = 0; //清除启动标志，为下次计时做准备
			}
		case JOIN_STATE_RUN_APP3:
			//switch is pressed, run app
			dstADDR.saddr = 0; //RFD sends to the coordinator
			my_timer = halGetMACTimer(); //timer must be initialized before entering PP_SEND_STATE
			Zigbee_Data_Service(); //only exits on if excessive misses
			//try rejoining network
			failures = 0;
			joinState = JOIN_STATE_START_REJOIN;
			break;
			//rejoin states only executed by RFD
#ifndef LRWPAN_COORDINATOR
		case JOIN_STATE_START_REJOIN:
			EVB_LED1_OFF();
			printf("尝试重新加入网络!\n");
			aplRejoinNetwork();
			joinState = JOIN_STATE_WAIT_FOR_REJOIN;
			break;

		case JOIN_STATE_WAIT_FOR_REJOIN:
			if (apsBusy())
				break; //if stack is busy, continue
			if (aplGetStatus() == LRWPAN_STATUS_SUCCESS) {
				failures = 0;
				EVB_LED1_ON();
				printf("网络重连成功！\n");
				printJoinInfo();
				joinState = JOIN_STATE_RUN_APP3; //don't wait for button press
			} else {
				failures++;
				if (failures == MAX_REJOIN_FAILURES) {
					//this starts everything over
					//conPrintROMString(" Max Rejoins failed, trying to join.\n");
					joinState = JOIN_STATE_START_JOIN;
				} else {
					//else, wait to try again
					printf("加入网络失败！等待2s重新尝试。\n");
					my_timer = halGetMACTimer();
					//wait for 2 seconds
					while ((halMACTimerNowDelta(my_timer))
							< MSECS_TO_MACTICKS(2 * 1000))
						;

					joinState = JOIN_STATE_START_REJOIN;
				}
			}
			break;
#endif

		default:
			joinState = JOIN_STATE_START_JOIN;

		}

	}
}

//########## Callbacks ##########

//callback for anytime the Zero Endpoint RX handles a command
//user can use the APS functions to access the arguments
//and take additional action is desired.
//the callback occurs after the ZEP has already taken
//its action.
LRWPAN_STATUS_ENUM usrZepRxCallback(void) {

#ifdef LRWPAN_COORDINATOR
	if (aplGetRxCluster() == ZEP_END_DEVICE_ANNOUNCE) {
		//a new end device has announced itself, print out the
		//the neightbor table and address map
		DEBUG_PRINTNEIGHBORS(DBG_INFO);
	}
#endif
	return LRWPAN_STATUS_SUCCESS;
}

//callback from APS when packet is received
//user must do something with data as it is freed
//within the stack upon return.

LRWPAN_STATUS_ENUM usrRxPacketCallback(void) {

#ifdef LRWPAN_COORDINATOR
	//UDP发送数据
	Zg_Data_Send();
	heart_startTime = halGetMACTimer();

#else//节点接收参数配置
	Zigbee_Recv_Settings();

#endif

	return LRWPAN_STATUS_SUCCESS;
}

#if 1//#ifdef LRWPAN_FFD
//Callback to user level to see if OK for this node
//to join - implement Access Control Lists here based
//upon IEEE address if desired
//节点认证，ptr为当前申请入网的节点物理地址，capinfo为设备类型信息
//这里需要用户自己添加节点过滤处理
#if STATIC_LADDR_MARK	//静态数组定义的方法
static const LADDR node_list[] = {

	{	0x01, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x02, 0x21, 0x01,
		0x00, 0x00, 0x4B, 0x12, 0x00}, {0x03, 0x21, 0x01, 0x00, 0x00,
		0x4B, 0x12, 0x00}, {0x04, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12,
		0x00}, {0x05, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {
		0x06, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x07, 0x21,
		0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x08, 0x21, 0x01, 0x00,
		0x00, 0x4B, 0x12, 0x00}, {0x09, 0x21, 0x01, 0x00, 0x00, 0x4B,
		0x12, 0x00},
	{	0x0a, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x0b, 0x21, 0x01,
		0x00, 0x00, 0x4B, 0x12, 0x00}, {0x0c, 0x21, 0x01, 0x00, 0x00,
		0x4B, 0x12, 0x00}, {0x0d, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12,
		0x00}, {0x0e, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {
		0x0f, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x10, 0x21,
		0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x11, 0x21, 0x01, 0x00,
		0x00, 0x4B, 0x12, 0x00}, {0x12, 0x21, 0x01, 0x00, 0x00, 0x4B,
		0x12, 0x00},
	{	0x13, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00}, {0x14, 0x21, 0x01,
		0x00, 0x00, 0x4B, 0x12, 0x00}, {0x15, 0x21, 0x01, 0x00, 0x00,
		0x4B, 0x12, 0x00}};
BOOL checkLongAddr(LADDR *ptr)
{
	for (UINT8 i = 0; i < (sizeof(node_list) / sizeof(LADDR)); i++) {
		for (UINT8 j = 0; j < sizeof(LADDR); j++) {
			if (ptr->bytes[j] != node_list[i].bytes[j])
			break;

			if (j == (sizeof(LADDR) - 1)) {
				return TRUE; //直接退出，返回为真
			}
		}
	}
	return FALSE;
}
#else	//地址掩码的方式进行范围过滤，所有在范围内的都通过
//B0    B1    B2    B3    B4    B5    B6    B7
//掩码,目前字节为单位处理，0--表示不关心，1--表示需要验证
const BYTE LADDR_MASK[8] = { 0x0, 0x0, 0x0, 0xff, 0xff, 0xff, 0xff, 0xff };
//样本
const BYTE LADDR_SAMP[8] = { 0x0, 0x21, 0x01, 0x00, 0x00, 0x4B, 0x12, 0x00 };

BOOL checkLongAddr(LADDR *ptr) {
	for (BYTE i = 0; i < 8; ++i) {
#if 0	//使用样本进行校验匹配
		if ((LADDR_MASK[i] & (ptr->bytes[i]))
				!= (LADDR_SAMP[i] & LADDR_MASK[i])) {
			return FALSE; //失败
		}
#else	//使用协调器自己的MAC地址进行校验
		uint8_t *addr = Board_Mac_Get_Ptr();
		if ((LADDR_MASK[i] & (ptr->bytes[i])) != (addr[i] & LADDR_MASK[i])) {
			return FALSE; //失败
		}
#endif
		if (i == 8)
			return TRUE; //成功
	}

}
#endif //endof STATIC_LADDR_MARK
BOOL usrJoinVerifyCallback(LADDR *ptr, BYTE capinfo) {

	BOOL ret;
	ret = checkLongAddr(ptr);
	if (ret == FALSE) { //错误打印
		conPrintROMString("Illegal Device ");
		conPrintLADDR(ptr);
		conPCRLF();
	}
	return ret;
}

BOOL usrJoinNotifyCallback(LADDR *ptr) {

	//allow anybody to join

	conPrintROMString("Node joined: ");
	conPrintLADDR(ptr);
	conPCRLF();
	DEBUG_PRINTNEIGHBORS(DBG_INFO);
	return TRUE;
}
#endif

//called when the slow timer interrupt occurs
#ifdef LRWPAN_ENABLE_SLOW_TIMER
void usrSlowTimerInt(void) {
}
#endif

//general interrupt callback , when this is called depends on the HAL layer.
void usrIntCallback(void) {
}
