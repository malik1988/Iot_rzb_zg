/*******************************************************************************
 * @文件    MCR20A_port.c
 * @作者  LiXianmao
 * @版本 V0.0.1
 * @日期    2016.9.13
 * @概述   mcr20a 与STM32 硬件接口（SPI接口，IRQ中断接口）移植层
 * @记注  接口中instance指定的为SPI端口，由于MCR20A固定为SPI1因此instance不需要，
 *        但是为了保证移植的兼容，instance还是保留了，不需要对其做任何处理。
 * @记注
 *******************************************************************************/

/*! *********************************************************************************
 *************************************************************************************
 * Include
 *************************************************************************************
 ********************************************************************************** */

#include "MCR20A_port.h"

#if 0
IRQ_Handler_t gMCR20_Handler;

#endif

uintn16_t spi_lowSpeed;
uint16_t spi_highSpeed;
/*! *********************************************************************************
 *************************************************************************************
 * Public functions
 *************************************************************************************
 ********************************************************************************** */
void spi_master_init(uint32_t instance) {
	//配置成Master模式，初始波特率设置为15MHz
	SPI_MCR20A_Init();
	//7.5MHz=SPI1总线频率60MHz/8
	SPI_MCR20A_Speed(SPI_BaudRatePrescaler_8);
	spi_lowSpeed = SPI_MCR20A_CR1_GET();
	//15MHz=SPI1总线频率60MHz/4
	SPI_MCR20A_Speed(SPI_BaudRatePrescaler_4);
	spi_highSpeed = SPI_MCR20A_CR1_GET();

	//配置CS引脚
	spi_cs_init();
}

/*****************************************************************************
 MCR20A芯片SPI工作频率：写最大18MHz，读最大9MHz
 为了保持兼容，mMCR20SpiWriteSpeed_c和mMCR20SpiReadSpeed_c不变，如下：
 #define mMCR20SpiWriteSpeed_c (16000000)
 #define mMCR20SpiReadSpeed_c   (8000000)
 但是实际STM32F207ZG SPI只能实现15MHz和7.5MHz
 mMCR20SpiWriteSpeed_c = 15MHz
 mMCR20SpiReadSpeed_c = 7.5MHz
 *****************************************************************************/
void spi_master_configure_speed(uint32_t instance, uint32_t freq) {
	uintn16_t prescaler;
#if 0
	if (freq > 8000000) {
		prescaler = SPI_BaudRatePrescaler_4; //60MHz/4=15MHz
	} else {
		prescaler = SPI_BaudRatePrescaler_8; //60MHz/8=7.5MHz
	}

	SPI_MCR20A_Speed(SPI_BaudRatePrescaler_8);
#else
	if (freq > 8000000) {
		SPI_MCR20A_CR1_SET(spi_highSpeed);
	} else {
		SPI_MCR20A_CR1_SET(spi_lowSpeed);
	}
#endif
}

/*****************************************************************************/
/*****************************************************************************/
void spi_master_transfer(uint32_t instance, uint8_t * sendBuffer,
		uint8_t * receiveBuffer, int16_t transferByteCount) {
	volatile uint8_t dummy;

	if (!transferByteCount)
		return;

	if (!sendBuffer && !receiveBuffer)
		return;

	//*可选*清SPI总线数据，保证没有数据

	//发送数据
	while (transferByteCount--) {
		//dummy=0x01;
		if (sendBuffer) {
			dummy = *sendBuffer;
			sendBuffer++;
		}

		dummy = SPI_MCR20A_WR(dummy);
		if (receiveBuffer) {
			*receiveBuffer = dummy;
			receiveBuffer++;
		}
	}
}
void spi_cs_init() {
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_MCR20A_CS_PIN_EN();

	GPIO_InitStructure.GPIO_Pin = SPI_MCR20A_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(SPI_MCR20A_CS_PORT, &GPIO_InitStructure);
}
//使能CS，低电平有效
void spi_cs_enable() {
	SPI_MCR20A_CS_LOW();
}

//禁用CS，高电平无效
void spi_cs_disable() {
	SPI_MCR20A_CS_HIGH();
}

//初始化RST管脚，MCR20 RST与STM32 RST的管脚直接相连
//因此，无法通过程序控制RST管脚控制MCR20A复位，该函数为空
void MCR20A_RST_Init() {
	GPIO_InitTypeDef GPIO_InitStructure;

	MCR20A_RESET_PIN_EN();
	GPIO_InitStructure.GPIO_Pin = MCR20A_RESET_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; //频率不用太高
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //硬件已经加了上拉电阻
	GPIO_Init(MCR20A_RESET_PORT, &GPIO_InitStructure);

}
//初始化IRQ中断引脚
void MCR20A_IRQ_Init() {
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	/*---配置IRQ引脚为输入--*/
	//使能GPIO的时钟
	MCR20A_IRQ_PIN_EN();

	GPIO_InitStructure.GPIO_Pin = MCR20A_IRQ_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //频率不用太高
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //硬件已经加了上拉电阻
	GPIO_Init(MCR20A_IRQ_PORT, &GPIO_InitStructure);

	/*---将IRQ引脚关联EXTI中断----*/
	//使能SYSCFG时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	//配置LINE
	MCR20A_IRQ_LINE_CFG();

	/* Configure EXTI Line */
	EXTI_InitStructure.EXTI_Line = MCR20A_IRQ_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; //初始化时不打开中断
	EXTI_Init(&EXTI_InitStructure);
	/* Enable and set EXTI Line0 Interrupt to the  priority */
	NVIC_InitStructure.NVIC_IRQChannel = MCR20A_IRQ_EXTI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
			MCR20A_IRQ_PRIO_EMPTION;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = MCR20A_IRQ_PRIO_SUB;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE; //初始化的时候不打开中断
	NVIC_Init(&NVIC_InitStructure);

	//先清中断标志，保证中断使能之前不会误触发中断。
	MCR20A_IRQ_CLEAR();
}

void MCR20A_IRQ_Disable() {
	NVIC_InitTypeDef NVIC_InitStructure;
#if 0
	EXTI_InitTypeDef EXTI_InitStructure;

	/* Configure EXTI Line */
	EXTI_InitStructure.EXTI_Line = MCR20A_IRQ_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStructure);
#endif

	/* Enable and set EXTI Line Interrupt to the  priority */
	NVIC_InitStructure.NVIC_IRQChannel = MCR20A_IRQ_EXTI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
			MCR20A_IRQ_PRIO_EMPTION;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = MCR20A_IRQ_PRIO_SUB;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
}
void MCR20A_IRQ_Enable() {
	NVIC_InitTypeDef NVIC_InitStructure;
#if 0
	EXTI_InitTypeDef EXTI_InitStructure;

	/* Configure EXTI Line */
	EXTI_InitStructure.EXTI_Line = MCR20A_IRQ_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
#endif

	/* Enable and set EXTI Line Interrupt to the  priority */
	NVIC_InitStructure.NVIC_IRQChannel = MCR20A_IRQ_EXTI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =
			MCR20A_IRQ_PRIO_EMPTION;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = MCR20A_IRQ_PRIO_SUB;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
#if 0	//不用该函数，在halstack.c定义
//中断入口
void MCR20A_IRQ_Entry()
{
	//清中断标志
	MCR20A_IRQ_CLEAR();
	//不为空则执行回调函数
	if(0!=gMCR20_Handler)
	gMCR20_Handler();
}
#endif

#ifdef MCR20A_MCU_RNG_ENABLE
//初始化RNG随机数发生器
void MCR20A_MCU_RNG_Init() {
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);
}
//获取RNG随机数值
uint8_t MCR20A_MCU_GetRNG() {
	uint8_t ret;
	while (RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET)
		; //等待随机数准备完毕
	ret = (uint8_t) RNG_GetRandomNumber(); //读数
}
#endif

