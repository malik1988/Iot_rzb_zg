/*
 V0.1 Initial Release   10/July/2006  RBR

 */

#include "hal.h"
#include "halStack.h"
#include "console.h"
//#include "debug.h"
#include "ieee_lrwpan_defs.h"
#include "memalloc.h"
#include "phy.h"
#include "mac.h"
#include "MCR20Reg.h"
#include "MCR20Drv.h"
#include "MCR20A_Phy.h"
#include "MCR20A_port.h"

#define MCR20A_PAN   0  //PAN0或PAN1，MCR20A具有双PAN功能
static volatile RADIO_FLAGS local_radio_flags; //记录射频模块工作模式状态
static volatile PHY_FREQ_ENUM freq_pre; //记录功能
static volatile BYTE channel_pre;
BYTE halRadioInited = 0; //射频模块初始化标志，0-未初始化，1-已经初始化
BYTE halSendStart = 0; //发送开始标志，1-发送开始，0-发送结束或未开始
BYTE txRetry = 1; //重发次数(实际总的发送次数，1表示只发送1次没有重发，2表示发送2次，3发送3次)

#if RX_AUTO_TIMEOUT_ENABLE//开启RX自动超时退出功能
//802.15.4协议中 一个数据等待最长约为5ms（包括ACK），因此这里设定为6ms
#define RX_TIMEOUT   375 //接收时序超时值,T3每16us一次计数，16us*375=6ms
#endif

//irq status，低字节在前。
typedef struct {
	BYTE seq :1; //任意时序结束
	BYTE tx :1; //tx时序结束
	BYTE rx :1; //rx时序结束
	BYTE cca :1; //CCA时序结束
	BYTE rxWaterMark :1; //接收数据个数超过设定阈值
	BYTE filterFail :1; //
	BYTE pllUnlock :1;
	BYTE rxFramePend :1; //接收

	BYTE wake :1; //唤醒
	BYTE pb_err :1; //数据包错误
	BYTE aes :1; //AES操作完成
	BYTE tmrStatus :1; //定时器状态
	BYTE pi :1; //接收到的包是否是数据请求
	BYTE srcaddr :1; //源地址匹配
	BYTE ccaBusy :1; //cca工作状态
	BYTE crcValid :1; //CRC有效标志

	BYTE tmr1 :1; //TMR1中断，1有效
	BYTE tmr2 :1; //TMR2中断，1有效
	BYTE tmr3 :1; //TMR3中断，1有效
	BYTE tmr4 :1; //TMR4中断，1有效
	BYTE tmr1msk :1; //TMR1中断使能，0有效
	BYTE tmr2msk :1; //TMR2中断使能，0有效
	BYTE tmr3msk :1; //TMR3中断使能，0有效
	BYTE tmr4msk :1; //TMR4中断使能，0有效
} irq_t;
irq_t irq;

#ifdef  LRWPAN_ASYNC_INTIO  //异步数据收发，采用缓冲处理
static volatile BYTE serio_rxBuff[LRWPAN_ASYNC_RX_BUFSIZE];
static volatile BYTE serio_rxHead, serio_rxTail;
#endif

void MCR20A_SetSeq(uint8_t seq) {
	BYTE phyReg;
	if (gIdle_c == seq || gTR_c == seq || gTX_c == seq || gRX_c == seq
			|| gCCA_c == seq) {

		phyReg = MCR20Drv_DirectAccessSPIRead(PHY_CTRL1);
		phyReg &= ~(cPHY_CTRL1_XCVSEQ);
		phyReg |= seq;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
	}
}

/*---------------------------------------------------------------------------
 * @名称: halInit
 * @概述: 初始化硬件设备
 * @输入: 无
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/
void halInit(void) {
	//Set clock source
	local_radio_flags.val = 0;

	//初始化协议栈打印输出端口
	halInitUart();
	//初始化硬件，初始化成MCR20A_PAN
	MCR20A_HwInit(MCR20A_PAN);

#ifdef LRWPAN_COORDINATOR
	//PAN_COORDINATOR 是否是协调器，协调器接收所有地址的数据，否则只接受自己为目标的数据
	//设置为PAN_COORDINATOR模式，接收所有地址的数据
	MCR20A_SetMacRole(PAN_COORDINATOR, MCR20A_PAN);
#else
	//只接受目标地址为自己的数据
	MCR20A_SetMacRole(PAN_ENDDIVCE, MCR20A_PAN);
#endif

#if DATA_ENCRYPTION_ENABLE //开启ASM硬件加密
	if (MCR20A_ASM_Selftest()) {
		conPrintROMString_func("Data Encryption Test OK !\n");
	} else {
		conPrintROMString_func(
				"Data Encryption Test FAILED ! **End Program** !\n");
		exit(-1); //如果开启了数据加密但是加密计算器自测试失败，则强制终止整个程序！
	}
#endif

}

/*---------------------------------------------------------------------------
 * @名称: halInitUart
 * @概述: 初始化UART设备
 * @输入: 无
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/
void halInitUart(void) {

#ifdef LRWPAN_ASYNC_INTIO
	serio_rxHead = 0;
	serio_rxTail = 0;
	//开启串口中断
	//INT_ENABLE_URX0(INT_ON);
#endif

}

#ifdef  LRWPAN_ASYNC_INTIO

/*---------------------------------------------------------------------------
 * @名称: halGetch
 * @概述: (缓冲方式)从串口中获取一个字节的数据
 *        (异步方式，直接读缓冲器，数据来源于中断服务)
 *        缓冲器采用循环结构，读取到了最尾部后再重新回到头。
 * @输入: 无
 * @输出: char型数据
 * @备注: 该模块函数与任何硬件无关，可以直接移植。
 *---------------------------------------------------------------------------*/
char halGetch(void) {
	char x;
	do {
		x = serio_rxHead; //use tmp because of volt decl
	}while (serio_rxTail == x);
	serio_rxTail++;
	if (serio_rxTail == LRWPAN_ASYNC_RX_BUFSIZE)
	serio_rxTail = 0;
	return (serio_rxBuff[serio_rxTail]);
}

/*---------------------------------------------------------------------------
 * @名称: halInitUart
 * @概述: (缓冲方式)UART是否有数据可读状态。
 *        只要当前缓冲的头标志跟尾部标志不等即认为缓冲有数据。
 * @输入: 无
 * @输出: BOOL型状态
 * @备注: 该模块函数与任何硬件无关，可以直接移植。
 *---------------------------------------------------------------------------*/
BOOL halGetchRdy(void) {
	char x;
	x = serio_rxHead;
	return (serio_rxTail != x);
}

#else

/*---------------------------------------------------------------------------
 * @名称: halGetch
 * @概述: (直接读取)从串口中获取一个字节的数据
 * @输入: 无
 * @输出: char型数据
 * @备注:
 *---------------------------------------------------------------------------*/
char halGetch(void) {
	char c;

	return c;
}

/*---------------------------------------------------------------------------
 * @名称: halGetchRdy
 * @概述: (直接读取)UART是否有数据可读状态。
 * @输入: 无
 * @输出: BOOL型状态
 * @备注:
 *---------------------------------------------------------------------------*/
BOOL halGetchRdy(void) {
	BOOL ret = 0;

	return ret;
}

#endif

/*---------------------------------------------------------------------------
 * @名称: halUtilMemCopy
 * @概述: 内存拷贝
 * @输入: 无
 * @输出: 无
 * @备注: 该模块函数与任何硬件无关，可以直接移植。
 *---------------------------------------------------------------------------*/
void halUtilMemCopy(BYTE *dst, BYTE *src, BYTE len) {
	while (len) {
		*dst = *src;
		dst++;
		src++;
		len--;
	}
}

/*---------------------------------------------------------------------------
 * @名称: halGetMACTimer
 * @概述: 获取MAC时钟
 *        获取MCR20A的时间值，用于计算CCA
 * @输入: 无
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/
UINT32 halGetMACTimer(void) {
	UINT32 ret;
#if MCR20A_TIMER_ENABLE
	ret = MCR20A_GetMACTimer();
#else
	ret = GetTimeMS();
#endif
	return ret;
}

#ifdef LRWPAN_COMPILER_NO_RECURSION

/*---------------------------------------------------------------------------
 * @名称: halISRGetMACTimer
 * @概述: 中断获取MAC时钟
 *        获取MCR20A的时间值，用于计算CCA
 * @输入: 无
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/

UINT32 halISRGetMACTimer(void)
{
	return halGetMACTimer();
}

#endif

/*---------------------------------------------------------------------------
 * @名称: halMacTicksToUs
 * @概述: 计算一个us需要的Ticks数
 * @输入: 无
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/
UINT32 halMacTicksToUs(UINT32 ticks) {
	return GetUsFromTicks(ticks);
}

/*---------------------------------------------------------------------------
 * @名称: halGetRandomByte
 * @概述: 获取一个字节的随机数
 * @输入: 无
 * @输出: UINT8型的随机数
 * @备注:
 *---------------------------------------------------------------------------*/
UINT8 halGetRandomByte(void) {
	UINT8 byte = 0;
	UINT32 u32 = 0; //存储32位数据

#if MCR20A_MCU_RNG_ENABLE //使用STM32_CPU 的RNG随机数发生器，能有效提高效率
	byte = MCR20A_MCU_GetRNG();
#else//使用MCR20A芯片自带的RNG发生器，建议不开启
	MCR20A_GetRandomNo((uint32_t *) &u32);
	byte = (UINT8) u32; //截取8位数据
#endif
	return (byte);
}

/*---------------------------------------------------------------------------
 * @名称: halPutch
 * @概述: 串口写一个字节的数
 * @输入: char型数据
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/

void halPutch(char c) {
	xUSART2_putchar(c);
}

/*---------------------------------------------------------------------------
 * @名称: halRawPut
 * @概述: 串口写一个字节的数(与halPutch等同)
 * @输入: char型数据
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/
void halRawPut(char c) {
	xUSART2_putchar(c);
}

/*---------------------------------------------------------------------------
 * @名称: halSetChannel
 * @概述: 设置2.4GHz下射频模块的通道
 * @输入: BYTE channel-通道
 * @输出: LRWPAN_STATUS_ENUM枚举类型的状态值
 * @备注: 在2.4G下通道已经决定了频率的具体值
 *---------------------------------------------------------------------------*/
LRWPAN_STATUS_ENUM halSetChannel(BYTE channel) {
	if ((channel < 11) || (channel > 26))
		return (LRWPAN_STATUS_PHY_FAILED);

	MCR20A_SetCurrentChannelRequest(channel, MCR20A_PAN); //默认采用PAN0

	return LRWPAN_STATUS_SUCCESS;
}

/*---------------------------------------------------------------------------
 * @名称: halSetRadioIEEEFrequency
 * @概述: 设置射频模块的频率和通道
 * @输入: BYTE *buf -地址存放首地址
 * @输出: 无
 * @备注: 1.大端存储地址 ； 2.如果空间紧张可以存放在FLAH中
 *---------------------------------------------------------------------------*/

LRWPAN_STATUS_ENUM halSetRadioIEEEFrequency(PHY_FREQ_ENUM frequency,
		BYTE channel) {
	//只支持2.4G
	if (PHY_FREQ_2405M != frequency)
		return LRWPAN_STATUS_PHY_FAILED; //直接退出

	return halSetChannel(channel);
}

/*---------------------------------------------------------------------------
 * @名称: halGetProcessorIEEEAddress
 * @概述: 从FLASH中获取IEEE地址
 * @输入: BYTE *buf -地址存放首地址
 * @输出: 无
 * @备注: 1.大端存储地址 ； 2.如果空间紧张可以存放在FLAH中
 *---------------------------------------------------------------------------*/

void halGetProcessorIEEEAddress(BYTE *buf) {
#if USE_AUTO_MAC
	Board_Mac_Get(buf);
#else
	buf[0] = aExtendedAddress_B0;
	buf[1] = aExtendedAddress_B1;
	buf[2] = aExtendedAddress_B2;
	buf[3] = aExtendedAddress_B3;
	buf[4] = aExtendedAddress_B4;
	buf[5] = aExtendedAddress_B5;
	buf[6] = aExtendedAddress_B6;
	buf[7] = aExtendedAddress_B7;
#endif
}

/*---------------------------------------------------------------------------
 * @名称: halSetRadioIEEEAddress
 * @概述: 将IEEE地址设置到MCR20A射频芯片
 * @输入: 无
 * @输出: 无
 * @备注: IEEE地址必须保证每个节点是唯一的
 *---------------------------------------------------------------------------*/

void halSetRadioIEEEAddress(void) {
	BYTE buf[8];
	halGetProcessorIEEEAddress(buf);

	MCR20A_SetLongAddr(buf, MCR20A_PAN);
}

/*---------------------------------------------------------------------------
 * @名称: halSetRadioPANID
 * @概述: 设置射频芯片的PANID值
 * @输入: 无
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/

void halSetRadioPANID(UINT16 panid) {
	MCR20A_SetPanId((uint8_t *) &panid, MCR20A_PAN);
}

/*---------------------------------------------------------------------------
 * @名称: halSetRadioShortAddr
 * @概述: 设置射频芯片的网络地址
 * @输入: 无
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/

void halSetRadioShortAddr(SADDR saddr) {
	MCR20A_SetShortAddr((uint8_t *) &saddr, MCR20A_PAN);
}

//return value of non-zero indicates failure
//Turn on Radio, initialize for RF mode
//assumes that auto-ack is enabled
//this function based on sppInit example from ChipCon
//also sets the IEEE address
//if listen_flag is true, then radio is configured for
//listen only (no auto-ack, no address recognition)

/*
 Eventually, change this so that auto-ack can be configured as
 on or off. When Coordinator is trying to start a network,
 auto-ack, addr decoding will be off as Coordinator will be doing an energy
 scan and detecting PAN collisions, and thus will not be doing
 any acking of packets. After the network is started, then
 will enable auto-acking.
 Routers and End devices will always auto-ack

 i
 */

/*---------------------------------------------------------------------------
 * @名称: halSetRadioShortAddr
 * @概述: 设置射频芯片的频率
 * @输入: 频率
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/

LRWPAN_STATUS_ENUM halInitRadio(PHY_FREQ_ENUM frequency, BYTE channel,
		RADIO_FLAGS radio_flags) {
	LRWPAN_STATUS_ENUM status;
	//频率有变化才设置频率值
	if (freq_pre != frequency || channel_pre != channel) {
		freq_pre = frequency;
		halSetRadioIEEEAddress();
		//设置射频模块的频率和通道
		status = halSetRadioIEEEFrequency(frequency, channel);
		if (status != LRWPAN_STATUS_SUCCESS)
			return (status);

#ifdef LRWPAN_COORDINATOR	//协调器默认进入接收时序
		BYTE phyReg;
		//进入RX时序
		phyReg = mStatusAndControlRegs[PHY_CTRL1_INDEX_c];
		phyReg &= ~(cPHY_CTRL1_XCVSEQ | cPHY_CTRL1_RXACKRQD);
		phyReg |= gRX_c;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
#if RX_AUTO_TIMEOUT_ENABLE
		//启动T3计时器，用于硬件自动超时退出RX时序

		phyTime_t timeOut = 0;
		MCR20Drv_DirectAccessSPIMultiByteRead(EVENT_TMR_LSB, (BYTE *) &timeOut,
				3);
		timeOut += RX_TIMEOUT;
		timeOut &= 0x0FFFFFF;
		MCR20Drv_DirectAccessSPIMultiByteWrite(T3CMP_LSB, (BYTE *) &timeOut, 3);
		/* enable autosequence stop by TC3 match */
		mStatusAndControlRegs[PHY_CTRL4_INDEX_c] |= cPHY_CTRL4_TC3TMOUT;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL4,
				mStatusAndControlRegs[PHY_CTRL4_INDEX_c]);
#endif//endof RX_AUTO_TIMEOUT_ENABLE
#endif//endof LRWPAN_COORDINATOR
	}
	halRadioInited = 1;

	return (LRWPAN_STATUS_SUCCESS);
}

/*---------------------------------------------------------------------------
 * @名称: doIEEE_backoff
 * @概述: 进行CCA冲突避免
 * @输入: 无
 * @输出: 无
 * @备注: 最好MCR20A硬件初始化时，设置为CCABeforeTx。
 *---------------------------------------------------------------------------*/

void doIEEE_backoff(void) {
	BYTE be, nb, tmp, rannum;
	UINT32 delay, start_tick;

	be = aMinBE;
	nb = 0;
	do {
		if (be) {
			//do random delay
			tmp = be;
			//compute new delay
			delay = 1;
			while (tmp) {
				delay = delay << 1; //delay = 2**be;
				tmp--;
			}
			rannum = halGetRandomByte() & (delay - 1); //rannum will be between 0 and delay-1
			delay = 0;
			while (rannum) {
				delay += SYMBOLS_TO_MACTICKS(aUnitBackoffPeriod);
				rannum--;
			} //delay = aUnitBackoff * rannum
			  //now do backoff
			start_tick = halGetMACTimer();
			while (halMACTimerNowDelta(start_tick) < delay)
				;
		}
		//check CCA
		//检查CCA状态，如果CCA结束，则立即退出
#if 0
		if (PIN_CCA)
		break;
#else

		MCR20Drv_DirectAccessSPIMultiByteRead(IRQSTS2,
				&mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c], 1);
		if (mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c] & cIRQSTS2_CCA) {
			//busy
		} else
			//CCA检查发现IDLE
			break;
#endif
		nb++;
		be++;
		if (be > aMaxBE)
			be = aMaxBE;
	} while (nb <= macMaxCSMABackoffs); //最大重试5次，[0,4]
	return;
}

#ifdef  LRWPAN_ASYNC_INTIO

/*---------------------------------------------------------------------------
 * @名称: urx0_service_IRQ
 * @概述: UART0中断服务程序
 * @输入: 无
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/INTERRUPT_FUNC urx0_service_IRQ(
		void) {

	BYTE x, y;

	serio_rxHead++;
	if (serio_rxHead == LRWPAN_ASYNC_RX_BUFSIZE)
	serio_rxHead = 0;
	x = serio_rxHead; //use tmp variables because of Volatile decl
	//y = U0DBUF; //获取数据
	serio_rxBuff[x] = y;
}

#endif

//This timer interrupt is the periodic interrupt for
//evboard functions
//不能使用，不支持
#undef LRWPAN_ENABLE_SLOW_TIMER
#ifdef LRWPAN_ENABLE_SLOW_TIMER

// assuming 16us period, have 1/16us = 62500 tics per seocnd
#define T2CMPVAL (62500/SLOWTICKS_PER_SECOND)

/*---------------------------------------------------------------------------
 * @名称: t2_service_IRQ
 * @概述: T2中断服务程序，任务调度使用
 * @输入: 无
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/
INTERRUPT_FUNC t2_service_IRQ(void)
{
	UINT32 t;

	INT_GLOBAL_ENABLE(INT_OFF);
	INT_SETFLAG_T2(INT_CLR); //clear processor interrupt flag
	//compute next compare value by reading current timer value, adding offset
	t = 0x0FF & T2OF0;
	t += (((UINT16)T2OF1) << 8);
	t += (((UINT32) T2OF2 & 0x0F) << 16);
	t += T2CMPVAL;//add offset
	T2PEROF0 = t & 0xFF;
	T2PEROF1 = (t >> 8) & 0xFF;
	//enable overflow count compare interrupt
	T2PEROF2 = ((t >> 16) & 0x0F) | 0x20;
	T2CNF = 0x03;//this clears the timer2 flags
	evbIntCallback();//Evaluation board callback
	usrSlowTimerInt();//user level interrupt callback
	INT_GLOBAL_ENABLE(INT_ON);
}
#endif


void halGetPanId(BYTE *pPanId)
{
	MCR20Drv_IndirectAccessSPIMultiByteRead((uint8_t) MACPANID0_LSB,
					pPanId, 2);
}

void halGetShortId(BYTE *id)
{
	MCR20Drv_IndirectAccessSPIMultiByteRead((uint8_t) MACSHORTADDRS0_LSB,
			id, 2);
}
//发送空闲状态，置射频模块为接收状态
void halIdle() {
	//halResetSeq();
}

//发送并接收ACK等待超时,退出ACK接收等待时序
void halTxTimeOut() {
	BYTE phyReg, xcvseq, rxAckRqd, seqState;

	//关闭中断
	MCR20Drv_IRQ_Disable();
	MCR20Drv_IRQ_Clear();

	phyReg = MCR20Drv_DirectAccessSPIRead(PHY_CTRL1);
	rxAckRqd = phyReg & (cPHY_CTRL1_RXACKRQD); //获取当前ACK等待标志

	if (rxAckRqd) { //cPHY_CTRL1_RXACKRQD 有效则表示当前处于ACK等待

		xcvseq = phyReg & (cPHY_CTRL1_XCVSEQ); //获取当前时序
		//当前是否处于接收状态
		if (gRX_c == xcvseq || gTR_c == xcvseq) {
			seqState = MCR20Drv_DirectAccessSPIRead(SEQ_STATE) & 0x1F;
			if (seqState) { //不为0，表示射频模块不为空闲状态

				if ((seqState <= 0x06) || (seqState == 0x15)
						|| (seqState == 0x16)) {
					//RX时序正在接收数据包，不处理任何时序，等待接收完成
					MCR20Drv_IRQ_Enable();
					return LRWPAN_STATUS_PHY_TX_START_FAILED;
				}

				//打断当前接收时序，为下面启动发送时序做准备
				phyReg &= ~(cPHY_CTRL1_XCVSEQ);
				phyReg |= gIdle_c;
				MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
				//等待时序终止完成
				while (MCR20Drv_DirectAccessSPIRead(SEQ_STATE) & 0x1F)
					;
			}
		}

		//清除射频模块中断寄存器，发送函数中产生的中断都不需要处理
		mStatusAndControlRegs[PHY_IRQSTS1_INDEX_c] =
				MCR20Drv_DirectAccessSPIMultiByteRead(IRQSTS2,
						&mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c], 2);
		//清中断标志
		MCR20Drv_DirectAccessSPIMultiByteWrite(IRQSTS1, mStatusAndControlRegs,
				3);

		//去除ACK等待
		phyReg &= ~(cPHY_CTRL1_XCVSEQ | cPHY_CTRL1_RXACKRQD);
		phyReg |= gRX_c;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
	}
	MCR20Drv_IRQ_Enable();
}

void halResetSeq() {
	MCR20Drv_IRQ_Disable();
	//MCR20Drv_IRQ_Clear();

	BYTE seqState = 0;
	seqState = MCR20Drv_DirectAccessSPIRead(SEQ_STATE) & 0x1F;

	//调试模式才用
	mStatusAndControlRegs[PHY_IRQSTS1_INDEX_c] =
			MCR20Drv_DirectAccessSPIMultiByteRead(IRQSTS2,
					&mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c], 7);

	BYTE phyReg;
	phyReg = MCR20Drv_DirectAccessSPIRead(PHY_CTRL1);

	//射频芯片正常工作起来是应该一直处于RX或TX状态，只有在中断处理函数中会处于空闲
	//万一芯片处于空闲但是又没有进入中断函数则需要该代码将其置为RX状态，并触发中断函数
	if ((0 == seqState)) {
		//进入RX时序
		phyReg = MCR20Drv_DirectAccessSPIRead(PHY_CTRL1);
		phyReg &= ~(cPHY_CTRL1_XCVSEQ | cPHY_CTRL1_RXACKRQD);
		phyReg |= gRX_c;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);

#if RX_AUTO_TIMEOUT_ENABLE
		//启动T3计时器，用于硬件自动超时退出RX时序

		phyTime_t timeOut = 0;
		MCR20Drv_DirectAccessSPIMultiByteRead(EVENT_TMR_LSB, (BYTE *) &timeOut,
				3);
		timeOut += RX_TIMEOUT;
		timeOut &= 0x0FFFFFF;
		MCR20Drv_DirectAccessSPIMultiByteWrite(T3CMP_LSB, (BYTE *) &timeOut, 3);
		/* enable autosequence stop by TC3 match */
		mStatusAndControlRegs[PHY_CTRL4_INDEX_c] |= cPHY_CTRL4_TC3TMOUT;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL4,
				mStatusAndControlRegs[PHY_CTRL4_INDEX_c]);
#endif

	}
	MCR20Drv_IRQ_Enable();
}

void evbPoll() {

}

void halRadioGetFrame(BYTE flen) {

	//BYTE ack_bytes[LRWPAN_ACKFRAME_LENGTH];
	BYTE ack;
	BYTE rssi;
	BYTE *ptr = NULL, *rx_frame = NULL;
	BYTE delayTmp = 50;

	if (5 > flen || flen > 127) //非法长度，直接退出，最小长度为ACK=5
		return;

	if (LRWPAN_ACKFRAME_LENGTH == flen) { //ACK
		//MCR20A自动校验ACK
		if (LRWPAN_GET_ACK_REQUEST(*(phy_pib.currentTxFrm))) {
			phyTxEndCallBack(); //清TX标识
		}
		ack = LRWPAN_ACKFRAME_LENGTH;
		macRxCallback(&ack, rssi);
	} else {
		if (!macRxBuffFull()) {
			//申请空间，实际需要的数据空间为flen-2+1
			rx_frame = MemAlloc(flen - 2 + 1);
			ptr = rx_frame;
		} else {
			//MAC RX buffer is full
		}
		if (ptr == NULL) {
			//无内存，不处理
		} else {
			//添加数据长度，1个字节
			*ptr = flen;
			ptr++;
			//MCR20A缓冲中CRC2字节硬件自动校验，不需要读取,实际读取长度flen-2
			MCR20Drv_PB_SPIBurstRead((uint8_t *) (ptr), flen - 2);

			rssi = MCR20Drv_DirectAccessSPIRead(LQI_VALUE);
			if (0 == rssi) {
				while (delayTmp--)
					; //等待一段时间，具体多少时间有待验证
				rssi = MCR20Drv_DirectAccessSPIRead(LQI_VALUE);
			}
			//将RSSI值从2的次方表示转换成 0~0XFF的数值 MCR20Drv_DirectAccessSPIRead(LQI_VALUE);不需要再处理
			rssi = MCR20A_LqiConvert(rssi);

			//处理PHY回调函数
			phyRxCallback();
			macRxCallback(rx_frame, rssi);
		}
	}
}

/*---------------------------------------------------------------------------
 * @名称: halShutdown
 * @概述: 关闭
 * @输入: 无
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/

void halShutdown(void) {
	//disable some interrupts
#ifdef LRWPAN_ENABLE_SLOW_TIMER
	INT_ENABLE_T2(INT_OFF);
#endif

	//关闭MCR20A中断
	//清零MCR20A中断
	//将MCR20A设置为IDLE状态
	MCR20A_Abort();

}

/*---------------------------------------------------------------------------
 * @名称: halWarmstart
 * @概述: 射频模块热启动
 * @输入: 无
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/

void halWarmstart(void) {
#if 0
	UINT32 myticks;
	//re-enable the timer interrupt
#ifdef LRWPAN_ENABLE_SLOW_TIMER
	INT_ENABLE_T2(INT_ON);
#endif
	//turn on the radio again
	RFPWR = RFPWR & ~(1 << 3);//RFPWR.RREG_RADIO_PD = 0;
	//wait for power to stabilize
	myticks = halGetMACTimer();
	while (halMACTimerNowDelta(myticks) < MSECS_TO_MACTICKS(10)) {
		//check the power up bit, max time is supposed to be 2 ms
		if (!(RFPWR & ~(1 << 4))) break;
	}

#endif
}

/*---------------------------------------------------------------------------
 * @名称: halSleep
 * @概述: 射频模块休眠
 * @输入: UINT32 msecs-休眠时间
 * @输出: 无
 * @备注:
 *---------------------------------------------------------------------------*/

void halSleep(UINT32 msecs) {
	//判定射频模块是否处于IDLE状态
	//如果是IDLE则进入休眠

	//关闭所有中断(射频相关)
	//清中断标志

	//设置射频模块的休眠定时器

	//设置射频模块为低功耗模式

	//打开射频模块的中断

}

//cPHY_CTRL2_SEQMSK开启SEQ中断，采用中断收发
//不能接收到ACK(ACK发送间隔在10ms之内，由硬件自动发送，接收响应不过来)
void IRQ_Handler() {

	BYTE xcvseq = 0, seqState;
	BYTE *ptr;
	BYTE tmpbuf[LRWPAN_MAX_FRAME_SIZE + 1]; //maximum payload size
	BYTE phyReg = 0, seqNext = gRX_c;
	BYTE flen = 0;
	phyTime_t timeOut = 0;

	/* Disable and clear transceiver(IRQ_B) interrupt */
	MCR20Drv_IRQ_Disable();
	MCR20Drv_IRQ_Clear();

	mStatusAndControlRegs[PHY_IRQSTS1_INDEX_c] =
			MCR20Drv_DirectAccessSPIMultiByteRead(IRQSTS2,
					&mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c], 7);
	//清中断标志
	MCR20Drv_DirectAccessSPIMultiByteWrite(IRQSTS1, mStatusAndControlRegs, 3);

	//获取当前中断类型
	halUtilMemCopy((BYTE *) &irq, mStatusAndControlRegs, sizeof(irq));
	//唤醒中断，射频芯片复位后默认开启
	if (irq.wake
			|| ((mStatusAndControlRegs[PHY_IRQSTS1_INDEX_c] == 0)
					&& (mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c] == 0))) {
		//直接退出中断不处理
		MCR20Drv_IRQ_Enable();
		return;
	}

	//获取当前时序控制器的状态
	xcvseq = mStatusAndControlRegs[PHY_CTRL1_INDEX_c] & cPHY_CTRL1_XCVSEQ;
	//保存数据长度
	flen = mStatusAndControlRegs[PHY_RX_FRM_LEN_INDEX_c];
	//保存时序状态
	phyReg = mStatusAndControlRegs[PHY_CTRL1_INDEX_c];

	/*-----------------------发送中断处理(必须比接收处理先执行)------------------------*/
	if (irq.tx) {
		if (irq.ccaBusy) //CCA检测到信道忙，则发送终止，需要重新发送
		{
			//PB数据缓冲区中还保存着上次的数据，不需要重新装填数据，直接启动TX时序即可完成开启发送
			if (txRetry > 0) //重发计数器递减
				txRetry--;
			if (txRetry && halSendStart) {
				//当前时序为发送时序，则重发数据只需要重写时序，数据已经在缓冲区中了。
				if (gTX_c == xcvseq || gTR_c == xcvseq) {
					MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
					MCR20Drv_IRQ_Enable();
					return; //退出
				}
			}
		} else { //发送成功
			if (LRWPAN_GET_ACK_REQUEST(*(phy_pib.currentTxFrm))) {
				//需要ACK确认
				//获取到ACK后才调用phyTxEndCallBack();
			} else { //不需要ACK
				phyTxEndCallBack();
			}
			halSendStart = 0; //发送结束
			txRetry = 0;
		}
	}

	/*-----------------------------接收数据处理--------------------------*/

	if (irq.rx) { //如果接收标志有效表示一个完整的数据包接收完成，直接处理
		halRadioGetFrame(flen);
	}

	seqState = MCR20Drv_DirectAccessSPIRead(SEQ_STATE) & 0x1F;
	/*-----------------------------时序结束处理--------------------------*/
	if (irq.seq || (seqState == 0)) { //时序正常结束，则进入接收状态
		phyReg &= ~(cPHY_CTRL1_XCVSEQ | cPHY_CTRL1_RXACKRQD); //去除ACK等待
		seqNext = gRX_c;
		phyReg |= seqNext;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
#if RX_AUTO_TIMEOUT_ENABLE
		//启动T3计时器，用于硬件自动超时退出RX时序

		MCR20Drv_DirectAccessSPIMultiByteRead(EVENT_TMR_LSB, (BYTE *) &timeOut,
				3);
		timeOut += RX_TIMEOUT;
		timeOut &= 0x0FFFFFF;
		MCR20Drv_DirectAccessSPIMultiByteWrite(T3CMP_LSB, (BYTE *) &timeOut, 3);
		/* enable autosequence stop by TC3 match */
		mStatusAndControlRegs[PHY_CTRL4_INDEX_c] |= cPHY_CTRL4_TC3TMOUT;
		MCR20Drv_DirectAccessSPIWrite(PHY_CTRL4,
				mStatusAndControlRegs[PHY_CTRL4_INDEX_c]);
#endif
	}

	MCR20Drv_IRQ_Enable();

}
void MCR20A_IRQ_Entry() {
	if (EXTI_GetITStatus(MCR20A_IRQ_EXTI_LINE) != RESET) {
		uint32_t us_pre, us_now;
		us_pre = GetTimeUS();
		IRQ_Handler();
		us_now = GetTimeUS();
		uint32_t delta = us_now - us_pre;
	}
}

//采用中断发送，该函数不做数据处理
LRWPAN_STATUS_ENUM halSendPacket(BYTE flen, BYTE *frm) {
	LRWPAN_STATUS_ENUM res;
	//total length, does not include length byte itself
	//last two bytes are the FCS bytes that are added automatically
	if (flen < 5 || flen + PACKET_FOOTER_SIZE > 127) {
		//packet size is too large!
		return LRWPAN_STATUS_PHY_TX_PKT_TOO_BIG;
	}

	BYTE xcvseq, seqState = 1; //初始seqState 不为0
	BYTE *ptr;
	BYTE tmpbuf[LRWPAN_MAX_FRAME_SIZE + 1]; //maximum payload size
	BYTE phyReg = 0, seqNext;
	BYTE txRetry = 0; //数据重发，1有效
	BYTE nextSeqEnable = 0; //启动下一个时序标识，1有效
	BYTE timeOutCount; //打断RX或者TR时序超时计数器

	phyTxStartCallBack();

	//关闭中断
	MCR20Drv_IRQ_Disable();
	MCR20Drv_IRQ_Clear();

	halSendStart = 1; //表示发送开始了
	phyReg = MCR20Drv_DirectAccessSPIRead(PHY_CTRL1);
	xcvseq = phyReg & (cPHY_CTRL1_XCVSEQ); //获取当前时序
	phyReg &= ~(cPHY_CTRL1_XCVSEQ); //

	//当前是否处于接收状态
	if (gRX_c == xcvseq || gTR_c == xcvseq) {
		seqState = MCR20Drv_DirectAccessSPIRead(SEQ_STATE) & 0x1F;
		if (seqState) { //不为0，表示射频模块不为空闲状态

			if ((seqState <= 0x06) || (seqState == 0x15)
					|| (seqState == 0x16)) {
				//RX时序正在接收数据包，不处理任何时序，等待接收完成
				MCR20Drv_IRQ_Enable();
				return LRWPAN_STATUS_PHY_TX_START_FAILED;
			}

			//打断当前接收时序，为下面启动发送时序做准备
			phyReg |= gIdle_c;
			MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);
			//等待时序终止完成
			while (MCR20Drv_DirectAccessSPIRead(SEQ_STATE) & 0x1F)
				;
		}
	}
	/*--------封装数据准备发送----*/
	ptr = &tmpbuf[0];
	*ptr = phy_pib.currentTxFlen + PACKET_FOOTER_SIZE; //length does not include length byte
	ptr++;
	//copy frame
	memcpy(ptr, phy_pib.currentTxFrm, phy_pib.currentTxFlen);

	//写数据到射频芯片
	MCR20Drv_PB_SPIBurstWrite(tmpbuf, phy_pib.currentTxFlen + 1);
	if (LRWPAN_GET_ACK_REQUEST(*(phy_pib.currentTxFrm))) {
		//需要ACK，直接采用TR时序，快速响应
		phyReg |= cPHY_CTRL1_RXACKRQD; //加上ACK等待
		seqNext = gTR_c;
		macTxCallback();
	} else {
		/* 启动TX时序 ，开启CCABFRTX*/
		seqNext = gTX_c;
		//普通的TX时序，在TX时序结束后才调用phyTxEndCallBack
	}

	//清除射频模块中断寄存器，发送函数中产生的中断都不需要处理
	mStatusAndControlRegs[PHY_IRQSTS1_INDEX_c] =
			MCR20Drv_DirectAccessSPIMultiByteRead(IRQSTS2,
					&mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c], 2);
	//清中断标志
	MCR20Drv_DirectAccessSPIMultiByteWrite(IRQSTS1, mStatusAndControlRegs, 3);

	phyReg |= seqNext;
	MCR20Drv_DirectAccessSPIWrite(PHY_CTRL1, phyReg);

	//打开中断
	MCR20Drv_IRQ_Enable();

	return LRWPAN_STATUS_SUCCESS;
}
//AES-CTR方式加密和解密数据
UINT8 halDataEncryptAndDecrypt(UINT8 key[16], UINT8 in[16], UINT8 cnt[16],
		UINT8 out[16]) {
	//每次新的加密都需要清除ASM缓冲区
	MCR20A_ASM_Clear();
	//调用CTR开启加密
	return MCR20A_CTR128_Encrypt(key, in, cnt, out);
}
