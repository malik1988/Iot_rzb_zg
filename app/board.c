/**
 *@file board.c
 *@brief RZG板硬件接口定义(MCU-STM32F207ZG)
 *@author lxm
 *@date 2016.10.13
 *@version 0.1.0
 *@par Copyright (c):
 *      XXX公司
 *@par 修改日志
 *      0.1.0 xx 2016.10.11 创建文件
 *@details 硬件清单请见board_config.h
 *@details !!!STM32F207没有硬件加密控制器!!!
 **/
#include "board.h"
#include "xprintf.h"

uint8_t debugFlag = 0; //打印信息标志，0--关闭打印，1--打开调试打印
uint32_t board_SW_Version; //软件版本号

/*-------------COM定义--------------------------*/
USART_TypeDef* COM_USART[COMn] = { EVAL_COM1, EVAL_COM2 };

GPIO_TypeDef* COM_TX_PORT[COMn] = { EVAL_COM1_TX_GPIO_PORT,
		EVAL_COM2_TX_GPIO_PORT };

GPIO_TypeDef* COM_RX_PORT[COMn] = { EVAL_COM1_RX_GPIO_PORT,
		EVAL_COM2_RX_GPIO_PORT };

const uint32_t COM_USART_CLK[COMn] = { EVAL_COM1_CLK, EVAL_COM2_CLK };

const uint32_t COM_TX_PORT_CLK[COMn] = { EVAL_COM1_TX_GPIO_CLK,
		EVAL_COM2_TX_GPIO_CLK };

const uint32_t COM_RX_PORT_CLK[COMn] = { EVAL_COM1_RX_GPIO_CLK,
		EVAL_COM2_RX_GPIO_CLK };

const uint16_t COM_TX_PIN[COMn] = { EVAL_COM1_TX_PIN, EVAL_COM2_TX_PIN };

const uint16_t COM_RX_PIN[COMn] = { EVAL_COM1_RX_PIN, EVAL_COM2_RX_PIN };

const uint8_t COM_TX_PIN_SOURCE[COMn] = { EVAL_COM1_TX_SOURCE,
		EVAL_COM2_TX_SOURCE };

const uint8_t COM_RX_PIN_SOURCE[COMn] = { EVAL_COM1_RX_SOURCE,
		EVAL_COM2_RX_SOURCE };

const uint8_t COM_TX_AF[COMn] = { EVAL_COM1_TX_AF, EVAL_COM2_TX_AF };

const uint8_t COM_RX_AF[COMn] = { EVAL_COM1_RX_AF, EVAL_COM2_RX_AF };

//串口输入使能
void Board_COM2_Input_Enable() {
	EVAL_COM2_EN_CLKEN();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;

	GPIO_InitStructure.GPIO_Pin = EVAL_COM2_EN_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(EVAL_COM2_EN_PORT, &GPIO_InitStructure);

	//低电平有效
	GPIO_ResetBits(EVAL_COM2_EN_PORT, EVAL_COM2_EN_PIN);

}
//串口输入禁用
void Board_COM2_Input_Disable() {
	//高电平禁用
	GPIO_SetBits(EVAL_COM2_EN_PORT, EVAL_COM2_EN_PIN);
}
/**
 * @brief  Configures COM port.
 * @param  COM: Specifies the COM port to be configured.
 *   This parameter can be one of following parameters:
 *     @arg COM1
 *     @arg COM2
 * @param  USART_InitStruct: pointer to a USART_InitTypeDef structure that
 *   contains the configuration information for the specified USART peripheral.
 * @retval None
 */
void Board_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct) {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM], ENABLE);

	if (COM == COM1) {
		/* Enable UART clock */
		RCC_APB2PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
	} else {
		/* Enable UART clock */
		RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
	}

	/* Connect PXx to USARTx_Tx*/
	GPIO_PinAFConfig(COM_TX_PORT[COM], COM_TX_PIN_SOURCE[COM], COM_TX_AF[COM]);

	/* Connect PXx to USARTx_Rx*/
	GPIO_PinAFConfig(COM_RX_PORT[COM], COM_RX_PIN_SOURCE[COM], COM_RX_AF[COM]);

	/* Configure USART Tx as alternate function  */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

	GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

	/* Configure USART Rx as alternate function  */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
	GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

	/* USART configuration */
	USART_Init(COM_USART[COM], USART_InitStruct);

	/* Enable USART */
	USART_Cmd(COM_USART[COM], ENABLE);
}

const uint16_t GPIO_PIN[LEDn] = { LED_ALARM_PIN, LED_RUN_PIN };
GPIO_TypeDef* GPIO_PORT[LEDn] = { LED_ALARM_PORT, LED_RUN_PORT };
const uint32_t GPIO_CLK[LEDn] = { LED_ALARM_CLK, LED_RUN_CLK };
/**
 * @brief  Configures LED GPIO.
 * @param  Led: Specifies the Led to be configured.
 *   This parameter can be one of following parameters:
 *     @arg ALARM
 *     @arg RUN
 * @retval None
 */
void Board_LEDInit(Led_TypeDef Led) {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable the GPIO_LED Clock */
	RCC_AHB1PeriphClockCmd(GPIO_CLK[Led], ENABLE);

	/* Configure the GPIO_LED pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN[Led];
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIO_PORT[Led], &GPIO_InitStructure);

	//初始化关闭LED灯
	Board_LedOff(Led);
}
void Board_LedOn(Led_TypeDef Led) {
	//低电平亮灯
	GPIO_ResetBits(GPIO_PORT[Led], GPIO_PIN[Led]);
}

void Board_LedOff(Led_TypeDef Led) {
	//高电平灭灯
	GPIO_SetBits(GPIO_PORT[Led], GPIO_PIN[Led]);
}
//初始化IO管脚
void Board_PWM_Pin_Init() {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable the GPIO_LED Clock */
	RCC_AHB1PeriphClockCmd(PWM_CLK, ENABLE);

	/* Configure the GPIO_LED pin */
	GPIO_InitStructure.GPIO_Pin = PWM_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PWM_PORT, &GPIO_InitStructure);
}

//使用PWM控制蜂鸣器，初始化管脚
void Board_PWM_Init() {
	Board_PWM_Pin_Init();

#if PWM_USE_TIM  //使用定时器产生PWM
	/*---初始化定时器TIM1----*/

	//使能定时器时钟
	PWM_TIM_CLK_EN();

	//TIM1/8 总线频率为30MHz ，TIM2~7总线频率为60MHz
	//计算频率=总线频率/(PWM_CLKDIV+1)/(PWM_PRESCALER+1)/PERIOD
	//目标PWM频率=4KHz，60M/4K=15000，使用IO高低电平翻转的话还要除以2，15000/2=7500,
	//定时器中断频率为60M/7500=8KHz
#define PWM_PERIOD  			(100-1)
#define PWM_PULSE   			(PWM_PERIOD/2)		//占空比为50%
#define PWM_PRESCALER  			(75-1)	//
#define PWM_CLKDIV				0
	/*-设置定时器频率-*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD; //一个周期对应的计数器值
	TIM_TimeBaseStructure.TIM_Prescaler = PWM_PRESCALER;
	TIM_TimeBaseStructure.TIM_ClockDivision = PWM_CLKDIV; //预分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数到中间值产生中断，用于翻转输出
	TIM_TimeBaseInit(PWM_TIM, &TIM_TimeBaseStructure);

	//自动装载周期计数值
	TIM_ARRPreloadConfig(PWM_TIM, ENABLE);
	//使能TIM
	TIM_Cmd(PWM_TIM, DISABLE); //初始化不开启定时器，使用PWM_ON函数开启

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = PWM_TIM_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
			PWM_TIM_IRQ_PRIO_EMPTION;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = PWM_TIM_IRQ_PRIO_SUB;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//先清除中断
	TIM_ClearITPendingBit(PWM_TIM, TIM_IT_Update);
	//使能TIM中断，TIM_IT_Update为溢出中断
	TIM_ITConfig(PWM_TIM, TIM_IT_Update, ENABLE);
#endif
}
#if PWM_USE_TIM  //使用定时器产生PWM
//开启PWM
void Board_PWM_On() {
	//使能TIM
	TIM_Cmd(PWM_TIM, ENABLE);
}
void Board_PWM_Off() {
	//使能TIM
	TIM_Cmd(PWM_TIM, DISABLE);
}
static uint8_t isLow = 0;
static uint32_t periodCount = 0, //蜂鸣器响一次维持的周期数，保证能听到就够100即可。
		delayCount = 0, //蜂鸣器响间隔时间计数，如果是1s的话则该值最大为4000
		beep1sCount = 0; //蜂鸣器响的次数，每次间隔时间1s
static uint32_t beep1sMax = 3; //蜂鸣器间隔1s响最大持续次数，

static uint8_t beepIsOn = 1; //0--蜂鸣器关闭，1--蜂鸣器正在响，初始为1保证第一次进入响
static uint8_t beepIsBusy = 0;
//修改蜂鸣器响一次维持的周期数，默认为3
ErrorStatus Board_PWM_SetCount(uint8_t c) {
	if (beepIsBusy || 0 == c)
		return ERROR; //失败，PWM还在工作，请等待停止
	beep1sMax = c; //设置蜂鸣器响周期数
	return SUCCESS;
}
//当前PWM是否正在工作，0--正在工作，1--已经停止
uint8_t Board_PWM_IsOff() {
	return beepIsBusy == 0 ? 1 : 0;
}

#define COUNT_1S		8000	//定时器频率为8KHz，1s对应的计数值为8000
#define COUNT_PERIOD    200		//200就可以听见，200/4K=50ms响
//定时器中断
void Board_PWM_IRQHandler() {
	TIM_ClearITPendingBit(PWM_TIM, TIM_IT_Update);
	beepIsBusy = 1;

	//COUNT_PERIOD个周期后停止，保证响的时间足够长人能听到
	if (periodCount > COUNT_PERIOD) {
		periodCount = 0;
		delayCount = 0;
		beepIsOn = 0; //关闭蜂鸣器
	}

	if (beepIsOn) {
		//输出PWM，蜂鸣器响
		if (isLow) {
			PWM_LOW();
			isLow = 0;
			periodCount++;
		} else {
			PWM_HIGH();
			isLow = 1;
		}
	} else {
		//开始计数
		delayCount++;
	}

	if (delayCount > 0 && beep1sMax == 1) { //只响1次直接停止
	//停止定时器，关闭蜂鸣器PWM
		TIM_Cmd(PWM_TIM, DISABLE);
		/*-还原所有计数器为初始值-*/
		periodCount = 0;
		delayCount = 0;
		beep1sCount = 0;
		beep1sMax = 3;
		beepIsOn = 1;
		beepIsBusy = 0; //标志蜂鸣器空闲
	} else {
		//1s时间到了
		if (delayCount > COUNT_1S) {
			beep1sCount++;
			//响次数到上限了
			if (beep1sCount >= beep1sMax) {
				//停止定时器，关闭蜂鸣器PWM
				TIM_Cmd(PWM_TIM, DISABLE);
				/*-还原所有计数器为初始值-*/
				periodCount = 0;
				delayCount = 0;
				beep1sCount = 0;
				beep1sMax = 3;
				beepIsOn = 1;
				beepIsBusy = 0; //标志蜂鸣器空闲
			} else {
				//重新打开蜂鸣器
				beepIsOn = 1;
			}
			delayCount = 0;
		}
	}
}
#else

//4.1KHz 的PWM波形，维持100个周期，即维持时间24ms
void Board_PWM_On() {
	uint16_t i, j;
	for (i = 0; i < 100; i++) {
		for (j = 0; j < 1200; j++)
		;
		PWM_LOW();
		for (j = 0; j < 1200; j++)
		;
		PWM_HIGH();
	}
}

#endif //ENDOF PWM_USE_TIM
#define RTC_CLOCK_SOURCE_LSE

void RTC_Config(void) {
	/* Enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC */
	PWR_BackupAccessCmd(ENABLE);

#if defined (RTC_CLOCK_SOURCE_LSI)  /* LSI used as RTC source clock*/
	/* The RTC Clock may varies due to LSI frequency dispersion. */
	/* Enable the LSI OSC */
	RCC_LSICmd(ENABLE);

	/* Wait till LSI is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
	{
	}

	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

#elif defined (RTC_CLOCK_SOURCE_LSE) /* LSE used as RTC source clock */
	/* Enable the LSE OSC */
	RCC_LSEConfig(RCC_LSE_ON);

	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {
	}

	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

#else
#error Please select the RTC Clock source inside the main.c file
#endif /* RTC_CLOCK_SOURCE_LSI */

	/* Enable the RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC APB registers synchronisation */
	RTC_WaitForSynchro();
}
//RTC备份区值，用于标志RTC设备是否初始化过了。
//当RTC设备初始化完成后将该值写入备份区，如果掉电丢失了则表示需要重新初始化RTC，否则直接读写RTC。
#define RTC_BKP_VALUE  0x32F2
//初始化RTC为1s一次中断
void Board_RTC_Init() {
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != RTC_BKP_VALUE) {
		RTC_Config();

		/*---初始化RTC时间--*/
		RTC_InitTypeDef RTC_InitStruct;
		RTC_StructInit(&RTC_InitStruct); //采用默认值进行初始化
		/* Check on RTC init */
		if (RTC_Init(&RTC_InitStruct) == ERROR) {
			//初始化失败
		}

		RTC_DateTypeDef RTC_DateStruct;
		RTC_DateStruct.RTC_Year = 16; //0~99
		RTC_DateStruct.RTC_Month = 11; //1~12
		RTC_DateStruct.RTC_Date = 1; //1~31
		RTC_DateStruct.RTC_WeekDay = 2; //
		if (RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct) == ERROR) {
			//设置日期失败
		}

		RTC_TimeTypeDef RTC_TimeStruct;
		RTC_TimeStruct.RTC_Hours = 11;
		RTC_TimeStruct.RTC_Minutes = 45;
		RTC_TimeStruct.RTC_Seconds = 0;
		RTC_TimeStruct.RTC_H12 = 0;
		if (RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct) == ERROR) {
			//设置时间失败
			printf("RTC 初始化失败");
		} else { //成功后写备份区，标志RTC初始化完成
			RTC_WriteBackupRegister(RTC_BKP_DR0, RTC_BKP_VALUE);
		}

	} else {

		/* Check if the Power On Reset flag is set */
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) {
			printf("\r\n Power On Reset occurred....\n");
		}
		/* Check if the Pin Reset flag is set */
		else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET) {
			printf("\r\n External Reset occurred....\n");
		}

		printf("\r No need to configure RTC....\n");

		/* Enable the PWR clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

		/* Allow access to RTC */
		PWR_BackupAccessCmd(ENABLE);

		/* Wait for RTC APB registers synchronisation */
		RTC_WaitForSynchro();

		/* Clear the RTC Alarm Flag */
		RTC_ClearFlag(RTC_FLAG_ALRAF);

		/* Clear the EXTI Line 17 Pending bit (Connected internally to RTC Alarm) */
		EXTI_ClearITPendingBit(RTC_IRQ_LINE_ALARM | RTC_IRQ_LINE_TS);

	}
#if 0

#if RTC_TAMP_ENABLE	//不能使用TAMP或STAMP
	//使能秒中断
	RTC_ITConfig(RTC_IT_TS, ENABLE);
	/*------------------
	 * RTC tamp和stamp使用相同的中断，
	 * 在服务程序中区分EXTI_GetITStatus
	 * --------------------*/
	//配置中断寄存器打开RTC_IRQ_LINE_TS中断
	EXTI_InitTypeDef EXTI_InitStructure;

	EXTI_InitStructure.EXTI_Line = RTC_IRQ_LINE_TS;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	//使能NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQ_CHANNEL_TS;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
	RTC_IRQ_PRIO_EMPTION_TS;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = RTC_IRQ_PRIO_SUB_TS;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#else
	RTC_AlarmTypeDef RTC_AlarmStructure;
	RTC_AlarmStructure.RTC_AlarmDateWeekDay = 0;
	RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = 0;
	RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = 30;
	RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_All
	& (~RTC_AlarmMask_Seconds); //只开秒钟
	RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);
	//使能秒中断
	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	/*------------------
	 * RTC tamp和stamp使用相同的中断，
	 * 在服务程序中区分EXTI_GetITStatus
	 * --------------------*/
	//配置中断寄存器打开RTC_IRQ_LINE_TS中断
	EXTI_InitTypeDef EXTI_InitStructure;

	EXTI_InitStructure.EXTI_Line = RTC_IRQ_LINE_ALARM;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	//使能NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQ_CHANNEL_ALARM;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
	RTC_IRQ_PRIO_EMPTION_ALARM;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = RTC_IRQ_PRIO_SUB_ALARM;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	//开启ALARMA
	RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
#endif //endof RTC_TAMP_ENABLE
#endif //
}
//RTC Tamp/Stamp中断服务程序
void RTC_IRQHandler_TS() {
	if (EXTI_GetITStatus(RTC_IRQ_LINE_TS) != RESET) { //STamp中断
		//清中断标志
		EXTI_ClearITPendingBit(RTC_IRQ_LINE_TS);

		//TODO 执行中断代码

	} else { //Tamp中断
		//TODO 清中断标志

	}
}
//RTC Alarm闹钟中断服务程序
void RTC_IRQHandler_ALARM() {

	EXTI_ClearITPendingBit(RTC_IRQ_LINE_ALARM);
}

//内置Flash初始化
//STM32F207内置512KB Flash，起始地址0x08000000，4*16KB+1*16KB+3*128KB
void Board_Flash_Init() {
	//FLASH_SetLatency(FLASH_Latency_3);
	FLASH_Unlock();
	FLASH_ClearFlag(
			FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR
					| FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR
					| FLASH_FLAG_PGSERR);
	//擦除整个数据块
	while (FLASH_EraseSector(USER_FLASH_SECTOR, VoltageRange_3)
			!= FLASH_COMPLETE)
		;
}
//从基地址开始写入MAC地址，长度8字节=8*8=64位
void Board_Flash_Write_Mac(uint64_t data) {
	uint32_t tmp;
	Board_Flash_Init();
	for (int i = 0; i < sizeof(data) / sizeof(tmp); i++) {
		tmp = (uint32_t)(data >> (i * 32));

		//FLASH_ProgramWord必须和VoltageRange_3匹配；
		//当VoltageRange_3时，只能使用FLASH_ProgramWord
		if (FLASH_ProgramWord(USER_FLASH_MAC_BASE + i * 4, tmp)
				== FLASH_COMPLETE) {
			//成功
		}
	}
	FLASH_Lock();
}
//读取FLASH中MAC地址，1--表示成功，0--表示失败
uint8_t Board_Flash_Read_Mac(uint8_t mac[8]) {
	uint8_t *ptr;
	ptr = (uint8_t *) (USER_FLASH_MAC_BASE);
	uint8_t addrValid = 0;

	for (int i = 0; i < 8; i++) {
		mac[i] = *ptr;
		ptr++;
		if (mac[i] != 0xFF) //不全为0xFF，则为有效
			addrValid++;
	}

	if (0 == addrValid) { //FLASH读取MAC地址失败，采用默认地址
		/*
		 mac[0] = aExtendedAddress_B0;
		 mac[1] = aExtendedAddress_B1;
		 mac[2] = aExtendedAddress_B2;
		 mac[3] = aExtendedAddress_B3;
		 mac[4] = aExtendedAddress_B4;
		 mac[5] = aExtendedAddress_B5;
		 mac[6] = aExtendedAddress_B6;
		 mac[7] = aExtendedAddress_B7;
		 */
		return 0; //失败
	}
	//成功
	return 1;
}

//读取MCU硬件ID号
void Board_MCU_ID(uint32_t id[3]) {
	uint32_t *ptr;
	ptr = (uint32_t *) MCU_ID_BASE;
	for (int i = 0; i < 3; i++) {
		id[i] = *ptr;
		ptr++;
	}
}

static uint8_t deviceMacAddr[12]; //设备序列号总长度为12字节
static uint8_t deviceMacOnce = 1;
//内部函数，自动生成MAC地址
void Board_Mac_AutoGen() {
	if (deviceMacOnce) { //只执行一次生成过程
		uint8_t *ptr = (uint8_t *) MCU_ID_BASE;
		//printf("Device ID: ");
		for (int i = 0; i < sizeof(deviceMacAddr); i++) {
			deviceMacAddr[i] = *ptr;
			//	printf("%02X", *ptr);
			ptr++;
		}
		deviceMacOnce = 0;
	}

	//printf("\n");

}
//用户接口，获取IEEE MAC地址8字节
void Board_Mac_Get(uint8_t mac[8]) {

	for (int i = 0; i < 8; i++) {
		mac[i] = deviceMacAddr[i];
	}
}

uint8_t* Board_Mac_Get_Ptr() {
	return deviceMacAddr;
}

//往Flash中写入数据，data为数据指针，size为写入数据的大小（大小必须是sizeof(uint32_t)的倍数）
void Board_Flash_Write(uint32_t *data, uint32_t size) {
	Board_Flash_Init();
	for (int i = 0; i < (size / sizeof(uint32_t)); i++) {
		//FLASH_ProgramWord必须和VoltageRange_3匹配；
		//当VoltageRange_3时，只能使用FLASH_ProgramWord
		FLASH_ProgramWord(USER_FLASH_BASE + i * 4, data[i]);
	}
	FLASH_Lock();
}

//将设备信息写入Flash
void Board_DeviceInfo_Write(Device_Info_t *info) {
	Board_Flash_Write((uint32_t *) (info), sizeof(Device_Info_t));
}

//将设备信息读出Flash，保存入info
void Board_DeviceInfo_Read(Device_Info_t *info) {
#if 0
	uint32_t *ptr=(uint32_t *)info;
	uint32_t *pflash=(uint32_t *)(USER_FLASH_BASE);
	for(uint32_t i=0;i<sizeof(Device_Info_t)/sizeof(uint32_t);i++)
	{
		ptr[i]=pflash[i];
	}
#else
	memcpy(info, (uint32_t *) (USER_FLASH_BASE), sizeof(Device_Info_t));
#endif
}
//获取所有设备信息
Device_Info_t * Board_DeviceInfo_Get() {
	return (Device_Info_t *) (USER_FLASH_BASE);
}
//打印所有设备信息
void Board_DeviceInfo_Print() {
	//调试打印满足才打开打印
	if (debugFlag) {
		Device_Info_t *ptr = Board_DeviceInfo_Get(); //获取设备信息

		printf("---------------DeviceInfo-----------------\n");
		//打印MAC地址
		printf("--MAC Address: ");
		for (uint8_t i = 0; i < sizeof(ptr->mac); i++) {
			printf("%02X", ptr->mac[i]);
		}
		printf("\n");

		//打印SN
		printf("--SN: %08d\n", ptr->sn);

		//打印版本号
		printf("--VER FW: ");
		Board_Version_Print(ptr->ver_fw);
		printf("--VER SW: ");
		Board_Version_Print(ptr->ver_sw);

		//打印网络配置参数
		printf("--NetWork Setting \n");
		printf("----|_type: %04x\n", ptr->sets.type);
		printf("----|_idle: %d\n", ptr->sets.idle);
		printf("----|_tx: %d\n", ptr->sets.tx);
		printf("----|_heart: %d\n", ptr->sets.heart);

		//打印调试模式
		printf("--DEBUG: %s\n", (ptr->debug == 1) ? "ON" : "OFF");

#if (BOOT_LOADER || RZG)	//BOOT_LOADER需要自动识别设备RZB或ZG
		//只有ZG才有网络地址打印
		if (Board_Is_RZB() == 0) {
			//打印IP
			printf("--IP Address: ");
			for (uint8_t i = 0; i < sizeof(ptr->udp.ip); i++) {
				printf("%d.", ptr->udp.ip[i]);
			}
			printf("\n");

			//打印子网掩码
			printf("--MASK Address: ");
			for (uint8_t i = 0; i < sizeof(ptr->udp.ip_mask); i++) {
				printf("%d.", ptr->udp.ip_mask[i]);
			}
			printf("\n");

			//打印网关地址
			printf("--GATEWAY Address: ");
			for (uint8_t i = 0; i < sizeof(ptr->udp.ip_gateway); i++) {
				printf("%d.", ptr->udp.ip_gateway[i]);
			}
			printf("\n");

			//目标端IP地址
			printf("--TARGET Address: ");
			for (uint8_t i = 0; i < sizeof(ptr->udp.ip_target); i++) {
				printf("%d.", ptr->udp.ip_target[i]);
			}
			printf("\n");
			//打印网络端口
			printf("--IP SEND PORT: %d\n", ptr->udp.send_port);
			printf("--IP RECV PORT: %d\n", ptr->udp.recv_port);
		}
#endif
		printf("---------------DeviceInfo END-------------\n");
	}
}

//判定当前网络参数是否有效，如果有效则保存并设置生效,0--设置失败，1--设置成功
uint8_t Board_Network_Set(Network_setting_t *settings) {
	Device_Info_t info_org;
	uint8_t idle_valid = 0, tx_valid = 0, heart_valid = 0;
	uint8_t ret = 0;
	//是否默认值参数有效
	if ((settings->type) & (TYPE_MASK_SET_DEFAULT)) {
		//还原成默认值，并直接退出
		Board_Network_SetDefault();
		//ZigBee信息立即生效
		Network_Config();
		return 1; //返回设置成功
	}

	Board_DeviceInfo_Read(&info_org); //获取板上现有参数
	if (((settings->type) & (TYPE_MASK_IDLE_TIME))
			&& (IS_VALID_IDLE_TIME(settings->idle))) { //空闲时间有效

		//修改原始参数
		info_org.sets.idle = settings->idle;

		idle_valid = 1;
	}
	if (((settings->type) & (TYPE_MASK_TX_TIMEOUT))
			&& (IS_VALID_TX_TIMEOUT(settings->tx))) { //空闲发送超时有效

		//保证接收到的参数必须满足tx<heart，
		//如果接收到的参数有效位中heart有效，则使用接收到的heart值进行比较
		//否则使用原始记录的heart值进行比较，只有满足条件才保存，否则不处理
		if ((settings->type) & (TYPE_MASK_HEART_TIMEOUT)) { //心跳参数有效位为1
			if (IS_VALID_HEART_TIMEOUT(settings->heart,settings->tx)) {
				//参数有效
				tx_valid = 1;
			}
		} else { //心跳参数有效位为0，采用原始心跳参数进行比较
			if (IS_VALID_HEART_TIMEOUT(info_org.sets.heart,settings->tx)) {
				//参数有效
				tx_valid = 1;
			}
		}

		if (tx_valid) {
			//修改原始参数
			info_org.sets.tx = settings->tx;
		}
	}
	//heart必须在tx处理之后
	if ((settings->type) & (TYPE_MASK_HEART_TIMEOUT)) { //空闲心跳超时有效

		if (IS_VALID_HEART_TIMEOUT(settings->heart,info_org.sets.tx)) //心跳超时时间>原始的发送超时
		{
			//修改原始参数
			info_org.sets.heart = settings->heart;
			heart_valid = 1;
		}

	}

	//写入Flash
	if (idle_valid || tx_valid || heart_valid) {
		info_org.sets.type = settings->type; //保存类型值

		Board_DeviceInfo_Write(&info_org);

		//配置网络，使得参数生效，该函数必须在Flash写入之后调用
		Network_Config();
		ret = 1; //设置成功

		//打印
		Dprintf("NetSetting Idle=%d\n", info_org.sets.idle);
		Dprintf("NetSetting Tx=%d\n", info_org.sets.tx);
		Dprintf("NetSetting Heart=%d\n", info_org.sets.heart);
		Dprintf("网络参数设置成功!\n");

	}

	return ret;
}

//ZigBee网络相关参数还原成默认值，不包括UDP参数设置
void Board_Network_SetDefault() {
	Device_Info_t info;
	uint8_t *mac = Board_Mac_Get_Ptr();
	Board_Mac_AutoGen(); //生成MAC地址

	info.flag = DEVICE_INFO_VALID; //置为有效
	//设置MAC地址
	for (uint8_t i = 0; i < sizeof(info.mac); i++) {
		info.mac[i] = mac[i];
	}
	//设置SN号
	info.sn = DEVICE_SN;
	//设置固件版本号
	Device_Info_t info_org;
	Board_DeviceInfo_Read(&info_org); //获取板上现有参数
	info.ver_fw = info_org.ver_fw;
	//设置版本号
	info.ver_sw = Board_Version_Get();

	//设置网络参数为默认值
	info.sets.type = DEFAULT_VALUE_SET_TYPE;
	info.sets.idle = DEFAULT_VALUE_SET_IDLE;
	info.sets.tx = DEFAULT_VALUE_SET_TX;
	info.sets.heart = DEFAULT_VALUE_SET_HEART;
	//写入Flash
	Board_DeviceInfo_Write(&info);
}

//将设备信息设置为默认值
void Board_DeviceInfo_SetDefault() {
	Device_Info_t info;
	uint8_t *mac = Board_Mac_Get_Ptr();
	Board_Mac_AutoGen(); //生成MAC地址

	info.flag = DEVICE_INFO_VALID; //置为有效
	//设置MAC地址
	for (uint8_t i = 0; i < sizeof(info.mac); i++) {
		info.mac[i] = mac[i];
	}
	//设置SN号
	info.sn = DEVICE_SN;

	//设置版本号
#if BOOT_LOADER
	//设置固件版本号
	info.ver_fw = Board_Version_Get();
	//设置软件版本号
	Device_Info_t info_org;
	Board_DeviceInfo_Read(&info_org);//获取板上现有参数
	info.ver_sw=info_org.ver_sw;
#else
	//设置软件版本号
	info.ver_sw = Board_Version_Get();
	//设置固件版本号
	Device_Info_t info_org;
	Board_DeviceInfo_Read(&info_org); //获取板上现有参数
	info.ver_fw = info_org.ver_fw;
#endif

	//设置网络参数为默认值
	info.sets.type = DEFAULT_VALUE_SET_TYPE;
	info.sets.idle = DEFAULT_VALUE_SET_IDLE;
	info.sets.tx = DEFAULT_VALUE_SET_TX;
	info.sets.heart = DEFAULT_VALUE_SET_HEART;

	//调试模式
	info.debug = 0; //默认不开启
#if (BOOT_LOADER || RZG)	//BOOT_LOADER需要自动识别设备RZB或ZG
			//只有ZG才有网络地址打印
			if (Board_Is_RZB() == 0) {

				//设置IP地址
				info.udp.ip[0] = BOARD_IP_0;
				info.udp.ip[1] = BOARD_IP_1;
				info.udp.ip[2] = BOARD_IP_2;
				info.udp.ip[3] = BOARD_IP_3;

				//设置子网掩码
				info.udp.ip_mask[0] = BOARD_NETMASK_0;
				info.udp.ip_mask[1] = BOARD_NETMASK_1;
				info.udp.ip_mask[2] = BOARD_NETMASK_2;
				info.udp.ip_mask[3] = BOARD_NETMASK_3;

				//设置网关地址
				info.udp.ip_gateway[0] = BOARD_GW_0;
				info.udp.ip_gateway[1] = BOARD_GW_1;
				info.udp.ip_gateway[2] = BOARD_GW_2;
				info.udp.ip_gateway[3] = BOARD_GW_3;

				//设置服务器IP地址
				info.udp.ip_target[0] = BOARD_IP_SERVER_0;
				info.udp.ip_target[1] = BOARD_IP_SERVER_1;
				info.udp.ip_target[2] = BOARD_IP_SERVER_2;
				info.udp.ip_target[3] = BOARD_IP_SERVER_3;

				//设置网络端口
				info.udp.send_port = UDP_SEND_PORT;
				info.udp.recv_port = UDP_RECV_PORT;
			}
#endif
	//写入Flash
	Board_DeviceInfo_Write(&info);
}

//初始化设备信息，如果Flash中设备信息则写入。
void Board_DeviceInfo_Init() {
	Device_Info_t *ptr = Board_DeviceInfo_Get();
	if (ptr->flag != DEVICE_INFO_VALID) { //未初始化
		//设置为默认值
		Board_DeviceInfo_SetDefault();
	}
	debugFlag = ptr->debug; //保存调试设置
#if BOOT_LOADER
			if (ptr->ver_fw != Board_Version_Get()) { //固件版本号未设置
				Device_Info_t info;
				Board_DeviceInfo_Read(&info);//获取板上现有参数
				info.ver_fw = Board_Version_Get();//写入固件版本号
				Board_DeviceInfo_Write(&info);
			}
#else

	if (ptr->ver_sw != Board_Version_Get()) { //软件版本号未设置
		Device_Info_t info;
		Board_DeviceInfo_Read(&info); //获取板上现有参数
		info.ver_sw = Board_Version_Get(); //写入软件版本号
		Board_DeviceInfo_Write(&info);
	}
#endif

}
//软件复位
void Board_Soft_Reset() {
	Dprintf("系统即将重启！\n");
	__set_FAULTMASK(1); //关闭所有中断，core_cmFunc.h
	NVIC_SystemReset(); //复位程序
}
//关闭回显的串口输入
uint32_t Board_COM2_Gets(uint8_t * buffP, uint32_t length) {
	static uint32_t bytes_read = 0;
	uint8_t c = 0;

	if (xUSART2_ValidInput()) {
		c = xUSART2_getchar();
		if (c == '\r') {
			//xUSART2_putchar('\n');
			//xUSART2_putchar('\r');
			buffP[bytes_read] = '\0';
			bytes_read = 0;

			return 1;
		}

		if (c == '\b') /* Backspace */
		{
			if (bytes_read > 0) {
				//xUSART2_putchar('\b');
				//xUSART2_putchar(' ');
				//xUSART2_putchar('\b');
				bytes_read--;
			}
			return 0;
		}
		//if (bytes_read >= (CMD_STRING_SIZE))
		if (bytes_read >= (length)) {
			//printf("Command string size overflow\r\n");
			bytes_read = 0;
			return 0;
		}
		if (c >= 0x20 && c <= 0x7E) {
			buffP[bytes_read] = c;
			bytes_read++;
			//xUSART2_putchar(c);
		}
	}

	return 0;
}

#if BOOT_LOADER

void Board_Shell() {
	uint8_t comKey[10];
	while (1) {
		printf("\nDEBUG>");
		//带回显的输入，阻塞方式
		xgets(comKey, sizeof(comKey));

		//调试模式开启
		if (strcmp(comKey, COM_KEY_SHELL_DEBUGON) == 0) {
			printf("\n调试模式开启.\n");
			Device_Info_t info_org;
			Board_DeviceInfo_Read(&info_org); //获取板上现有参数
			info_org.debug=1;//调试模式置为1

			//保存参数
			Board_DeviceInfo_Write(&info_org);
			//break;
		}
		//调试模式关闭
		if (strcmp(comKey, COM_KEY_SHELL_DEBUGOFF) == 0) {
			printf("\n调试模式关闭.\n");
			Device_Info_t info_org;
			Board_DeviceInfo_Read(&info_org); //获取板上现有参数
			info_org.debug=0;//调试模式置为1

			//保存参数
			Board_DeviceInfo_Write(&info_org);
			//break;
		}

		//恢复出厂设置
		if (strcmp(comKey, COM_KEY_SHELL_RESET) == 0) {

			printf("\n即将执行恢复出厂设置，[确认]请输入Y，其他不执行！\n");

			xgets(comKey, sizeof(comKey));
			if ((strcmp(comKey, "Y") == 0) || (strcmp(comKey, "y") == 0)) {
				printf("正在执行恢复出厂设置。\n");
				Board_DeviceInfo_SetDefault();
			} else {
				printf("出厂设置操作取消。\n");
			}
			//break;
		}

		//退出Shell
		if (strcmp(comKey, COM_KEY_SHELL_EXIT) == 0) {
			break;
		}

		//Xmodem
		if (strcmp(comKey, COM_KEY_SHELL_XUPDATE) == 0) {
			printf("\n开始启动Xmodem 1K下载升级程序，请勿输入任何字符！升级成功后将自动运行程序。\n");
			if (IAP_Service(0) == 0) { //IAP烧写失败，需要重试
				printf("程序升级失败！\n");
				//不退出while
			} else {
				printf("程序升级成功！即将运行应用程序。\n");
				break;
			}
		}
		//强制擦除应用程序
		if (strcmp(comKey, COM_KEY_SHELL_ERASE) == 0) {
			printf("！警告！即将强制擦除应用程序！，确认请输入Y，其他不执行。\n");
			xgets(comKey, sizeof(comKey));

			if ((strcmp(comKey, "Y") == 0) || (strcmp(comKey, "y") == 0)) {
				IAP_Program_Erase();
			}
			//break;
		}

		//打印设备信息
		if (strcmp(comKey, COM_KEY_SHELL_PRINTINFO) == 0) {
			Board_DeviceInfo_Init();
			debugFlag=1; //开启打印标志
			Board_DeviceInfo_Print();
			debugFlag=0;//关闭打印标志
			//break;
		}

		//设置设备信息
		if (strcmp(comKey, COM_KEY_SHELL_SETINFO) == 0) {
			printf("设备信息设置功能暂时不支持\n");
			//break;
		}

		//打印帮助
		if (strcmp(comKey, COM_KEY_SHELL_HELP) == 0) {
			printf(COM_KEY_SHELL_HELP_INFO);
			//break;
		}
	}
}

//进入SHELL，wait--用户等待标志，如果wait=0表示不等待立刻进入SHELL，否则等待一段时间用户输入密码正确则进入shell
void Board_Shell_Entry(uint8_t wait) {
	//使能串口输入
	Board_COM2_Input_Enable();
	if (wait) { //wait有效标志需要等待，如果等待超时则退出

		//等待5s中等待用户输入
#define COM_INPUT_TIMEOUT		5000
		uint32_t timeout = GetTimeMS();
		//串口输入"232start"才可以进入配置环境
		uint8_t comKey[10];

		printf("系统启动中，请等待%d秒.\n加载进度：  ", COM_INPUT_TIMEOUT / 1000);
		while ((GetTimeMS() - timeout) < COM_INPUT_TIMEOUT) {
			//不带回显的输入
			Board_COM2_Gets(comKey, sizeof(comKey));

			if (strcmp(comKey, COM_KEY_SHELL) == 0) {
				Board_Shell();
				//退出while
				break;
			}

			uint32_t delta = (GetTimeMS() - timeout);
			if (delta % (COM_INPUT_TIMEOUT / 10) == 0) {
				printf("\b\b%2d%", delta * 100 / COM_INPUT_TIMEOUT); //退格
			}
			//延迟一段时间，时间值
			delayMS(1);
		}
		printf("\b\b100\n");
	} else {
		Board_Shell();
	}
	//禁用串口输入
	Board_COM2_Input_Disable();
}

#endif //ENDOF BOOT_LOADER
//Flash程序保护，防止从芯片中读取程序
void Board_Flash_Protect() {
	if (FLASH_OB_GetRDP() == RESET) { //FLASH未读保护
		FLASH_Unlock();
		FLASH_OB_Unlock();
		FLASH_OB_RDPConfig(OB_RDP_Level_1); //写保护
		FLASH_OB_Launch(); //立即生效
		FLASH_OB_Lock();
		FLASH_Lock();
	}
}

//RAW原始数据获取方法，非阻塞方式读取串口数据，返回成功读取字节数
uint32_t Board_COM_Nowait_RAW_Gets(uint8_t *key, uint32_t len) {
	static uint32_t bytes = 0;
	if (xUSART2_ValidInput()) {
		key[bytes++] = xUSART2_getchar();
		if (bytes >= len) {
			bytes = 0;
			return len;
		}
	}
	return bytes;
}

#if RZB	//以下功能为RZB独有，串口读写RFID数据
//RFID接口采用UART5 波特率115200
void Board_Rfid_Init() {

	/*----初始化IF（Interface接口）-----*/
	RFID_IF_CLK_EN();
	//关联RX/TX引脚
	GPIO_PinAFConfig(RFID_IF_RX_PORT, RFID_IF_RX_SRC, RFID_IF_AF);
	GPIO_PinAFConfig(RFID_IF_TX_PORT, RFID_IF_TX_SRC, RFID_IF_AF);

	//初始化相关IO引脚
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

	/*----初始化RX-----*/

	RFID_IF_RX_CLK_EN();
	GPIO_InitStructure.GPIO_Pin = RFID_IF_RX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RFID_IF_RX_PORT, &GPIO_InitStructure);
	//初始化TX
	RFID_IF_TX_CLK_EN();
	GPIO_InitStructure.GPIO_Pin = RFID_IF_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RFID_IF_TX_PORT, &GPIO_InitStructure);

	/*----设置波特率-----*/
	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = 115200; //115200 230400 460800（UART5 APB1 30MHZ最高使用460800，再高出错数据概率>1%）
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(RFID_IF, &USART_InitStructure);
	USART_Cmd(RFID_IF, ENABLE);
}

//非阻塞方式读取串口数据，返回成功读取字节数
uint16_t Board_Rfid_Nowait_Gets(uint8_t *key, uint16_t len) {
	static uint16_t bytes = 0;
	while (USART_GetFlagStatus(RFID_IF, USART_FLAG_RXNE) != RESET) {
		key[bytes++] = (uint8_t)RFID_IF->DR & 0xFF;
		if (bytes >= len) {
			bytes = 0;
			return len;
		}
	}
	return bytes;
}
#endif //endof RZB
//识别当前硬件是否是RZB，1--是RZB ，0--是ZG
uint8_t Board_Is_RZB() {
	uint8_t state = 0; //1--是RZB ，0--是ZG
#define RZG_CHECK_PIN			GPIO_Pin_0	//PE0
#define RZB_CHECK_PORT			GPIOE		//PE
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	/*----初始化PE0引脚-----*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = RZG_CHECK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RZB_CHECK_PORT, &GPIO_InitStructure);

	//RZB PE0为悬空状态，内部上拉后读取的值应该为高电平
	//ZG PE0为接地状态，读取的值应该为低电平
	for (int i = 0; i < 100; i++) { //重复100次读取
		state = GPIO_ReadInputDataBit(RZB_CHECK_PORT, RZG_CHECK_PIN);
	}
	return state;
}

int aoti(char *str) {
	int ret = 0;
	while (*str != 0) {
		ret = ret * 10;
		ret += *str - '0';
		str++;
	}
	return ret;
}
//生成版本号
void Board_Version_Init() {
	const uint8_t month_table[][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	uint8_t str_year[5], str_month[4], str_day[3]; //年实际2016为4字节还有一个结束符所以为5字节
	uint8_t str_date[] = __DATE__;

	memcpy(str_month, str_date, (sizeof(str_month) - 1));
	str_month[(sizeof(str_month) - 1)] = 0; //追加结束符'\0'
	//查找月份
	uint8_t month = 0;
	for (int i = 0; i < (sizeof(month_table) / 4); i++) {
		if (strcmp(month_table[i], str_month) == 0) {
			month = i + 1;
			break;
		}
	}

	memcpy(str_day, str_date + 4, (sizeof(str_day) - 1));
	str_day[(sizeof(str_day) - 1)] = 0; //追加结束符'\0'

	memcpy(str_year, str_date + 7, (sizeof(str_year) - 1));
	str_year[(sizeof(str_year) - 1)] = 0; //追加结束符'\0'

	int year;
	year = aoti(str_year);

	int day;
	day = aoti(str_day);

	//低8位为DAY，紧接着4位为month，从低12位开始为年,年占至少12位
	board_SW_Version = (year << 12) | (month << 8) | day;

}
void Board_Version_Print(uint32_t ver) {
	int year, month, day;
	year = (ver >> 12) & 0xfff;
	month = (ver >> 8) & 0xf;
	day = ver & 0xff;

	printf("%d%d%d\n", year, month, day);
}

uint32_t Board_Version_Get() {
	return board_SW_Version;
}
//检查版本号是否合法，1--合法，0--不合法
uint8_t Board_Version_Check(uint32_t ver) {

	uint32_t tmp=(ver & BOARD_VERSION_MASK);
	if ((tmp> 0) && (tmp<BOARD_VERSION_MASK))
		return 1;
	else
		return 0;
}
