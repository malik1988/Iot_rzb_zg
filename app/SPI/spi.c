/**
 *@file spi.c
 *@brief STM32 SPI驱动代码
 *@details STM32 SPI 驱动
 *@author lxm
 *@date 2016.10.11
 *@version 0.1.0
 *@par Copyright (c):
 *      XXX公司
 *@par 修改日志
 *      0.1.0 xx 2016.10.11 创建文件
 **/

#include "spi.h"

//存储SPIX的配置参数
static SPI_InitTypeDef SPI_InitStructure[3];

/**
 *@name SPI1_Init
 *@brief 初始化SPI1,未含CS初始化配置，需要单独配置CS。
 *@detials 
 *@param[in] 无
 *@param[out] 无
 *@retval[OK] 成功
 *@retval[ERROR] 失败
 **/
void SPI1_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	//配置AHB_ENABLE
	SPI1_AHB_EN();

	//配置CLK_ENABLE
	SPI1_CLK_EN();

	//配置PORT

	GPIO_PinAFConfig(SPI1_SCK_PORT, SPI1_SCK_SOURCE, GPIO_AF_SPI1);
	GPIO_PinAFConfig(SPI1_MISO_PORT, SPI1_MISO_SOURCE, GPIO_AF_SPI1);
	GPIO_PinAFConfig(SPI1_MOSI_PORT, SPI1_MOSI_SOURCE, GPIO_AF_SPI1);

	//配置PIN
	//SCK
	GPIO_InitStructure.GPIO_Pin = SPI1_SCK ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉;
	GPIO_Init(SPI1_SCK_PORT, &GPIO_InitStructure);
	//MISO
	GPIO_InitStructure.GPIO_Pin = SPI1_MISO;
	GPIO_Init(SPI1_MISO_PORT, &GPIO_InitStructure);
	//MOSI
	GPIO_InitStructure.GPIO_Pin = SPI1_MOSI;
	GPIO_Init(SPI1_MOSI_PORT, &GPIO_InitStructure);


	//配置SPI1
	SPI_InitStructure[0].SPI_Direction = SPI_Direction_2Lines_FullDuplex; //设置为两线全双工
	SPI_InitStructure[0].SPI_Mode = SPI_Mode_Master; //设置为主模式
	SPI_InitStructure[0].SPI_DataSize = SPI_DataSize_8b; //SPI发送接收8位帧结构
	SPI_InitStructure[0].SPI_CPOL = SPI_CPOL_Low; //串行时钟在不操作时，时钟为低电平
	SPI_InitStructure[0].SPI_CPHA = SPI_CPHA_1Edge; //第一个时钟沿开始采样数据
	SPI_InitStructure[0].SPI_NSS = SPI_NSS_Soft; //NSS信号由软件（使用SSI位）管理
	SPI_InitStructure[0].SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; //SPI波特率预分频值为8
	SPI_InitStructure[0].SPI_FirstBit = SPI_FirstBit_MSB; //数据传输从MSB位开始
	SPI_InitStructure[0].SPI_CRCPolynomial = 7; //CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure[0]); //根据SPI_InitStruct中指定的参数初始化外设SPI2寄存器

	//使能SPI1
	SPI_Cmd(SPI1, ENABLE);

}

/**
 *@name SPI2_Init
 *@brief 初始化SPI2
 *@detials 
 *@param[in] 无
 *@param[out] 无
 *@retval[OK] 成功
 *@retval[ERROR] 失败
 **/
void SPI2_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	//配置AHB_ENABLE
	SPI2_AHB_EN();

	//配置CLK_ENABLE
	SPI2_CLK_EN();

	//配置PORT
	GPIO_PinAFConfig(SPI2_GPIO_PORT, SPI2_SCK_SOURCE, GPIO_AF_SPI2);
	GPIO_PinAFConfig(SPI2_GPIO_PORT, SPI2_MISO_SOURCE, GPIO_AF_SPI2);
	GPIO_PinAFConfig(SPI2_GPIO_PORT, SPI2_MOSI_SOURCE, GPIO_AF_SPI2);

	//配置PIN
	GPIO_InitStructure.GPIO_Pin = SPI2_SCK | SPI2_MISO | SPI2_MOSI;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //GPIO_PuPd_NOPULL;
	GPIO_Init(SPI2_GPIO_PORT, &GPIO_InitStructure);

	//配置SPI2
	SPI_InitStructure[1].SPI_Direction = SPI_Direction_2Lines_FullDuplex; //设置为两线全双工
	SPI_InitStructure[1].SPI_Mode = SPI_Mode_Master; //设置为主模式
	SPI_InitStructure[1].SPI_DataSize = SPI_DataSize_8b; //SPI发送接收8位帧结构
	SPI_InitStructure[1].SPI_CPOL = SPI_CPOL_Low; //串行时钟在不操作时，时钟为低电平
	SPI_InitStructure[1].SPI_CPHA = SPI_CPHA_1Edge; //第一个时钟沿开始采样数据
	SPI_InitStructure[1].SPI_NSS = SPI_NSS_Soft; //NSS信号由软件（使用SSI位）管理
	SPI_InitStructure[1].SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; //SPI波特率预分频值为8，SPI2总线30MHZ
	SPI_InitStructure[1].SPI_FirstBit = SPI_FirstBit_MSB; //数据传输从MSB位开始
	SPI_InitStructure[1].SPI_CRCPolynomial = 7; //CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure[1]); //根据SPI_InitStruct中指定的参数初始化外设SPI2寄存器

	//使能SPI2外设
	SPI_Cmd(SPI2, ENABLE);
}

/**
 *@name SPI2_WriteReadByte
 *@brief SPI2写一个字节数据后读取一个字节数据
 *@detials 
 *@param[in] 无
 *@param[out] 无
 *@retval[OK] 成功
 *@retval[ERROR] 失败
 **/
#if 0
u8 SPIx_WriteReadByte(SPI_TypeDef* SPIx, u8 TxData) {
	u8 retry = 0;
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET) //发送缓存标志位为空
	{
		retry++;
		if (retry > 200)
			return 0;
	}
	/* Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(SPIx, TxData); //发送一个数据
	retry = 0;
	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET) //接收缓存标志位不为空
	{
		retry++;
		if (retry > 200)
			return 0;
	}
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPIx); //返回接收数据
}
#else
u8 SPIx_WriteReadByte(SPI_TypeDef* SPIx, u8 TxData) {
	u8 ret;
	while(RESET==(SPIx->SR &SPI_I2S_FLAG_TXE));
	SPI_I2S_SendData(SPIx, TxData); //发送一个数据

	while(RESET==(SPIx->SR &SPI_I2S_FLAG_RXNE));
	ret=SPI_I2S_ReceiveData(SPIx); //返回接收数据
	return ret;
}
#endif

u8 SPIx_Read(SPI_TypeDef* SPIx) {
	u8 retry = 0;
	/* Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET) //接收缓存标志位不为空
	{
		retry++;
		if (retry > 200)
			return 0;
	}
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPIx);
}

/**
 *@name SPIx_PowerOn
 *@brief 使能SPIx的总线CLK从而打开SPIx
 *@detials
 *@param[in] 无
 *@param[out] 无
 *@retval[OK] 成功
 *@retval[ERROR] 失败
 **/
void SPIx_PowerOn(SPI_TypeDef* SPIx) {
	if (SPIx == SPI1)
		SPI1_CLK_EN();
	if (SPIx == SPI2)
		SPI2_CLK_EN();
}

/**
 *@name SPIx_PowerOff
 *@brief 禁用SPIx的总线CLK从而关闭SPIx
 *@detials
 *@param[in] 无
 *@param[out] 无
 *@retval[OK] 成功
 *@retval[ERROR] 失败
 **/
void SPIx_PowerOff(SPI_TypeDef* SPIx) {
	if (SPIx == SPI1)
		SPI1_CLK_DEN();
	if (SPIx == SPI2)
		SPI2_CLK_DEN();
}
/**
 *@name SPIx_MasterSpeed
 *@brief 配置SPIx主机时钟频率（只限对master模式有效）
 *@detials  **STM32 SPI硬件决定了SPI时钟频率**
 *@detials  SPI1--挂载APB2总线（APB2总线最高60MHz）上，SPI1时钟最高30MHz
 *@detials  SPI2/3--挂载APB1总线（APB1总线最高30MHz）上，SPI2/3时钟最高15MHz
 *@detials  SPI1/2/3--分频值最低为2
 *@param[in] SPIx--SPI1/2/3
 *@param[in] prescaler--分频值
 *@retval[0] 成功
 *@retval[-1] 失败
 **/
int SPIx_MasterSpeed(SPI_TypeDef* SPIx, uint16_t prescaler) {
	SPI_InitTypeDef *spi_init;

	//异常分频值直接退出
	if (!IS_SPI_BAUDRATE_PRESCALER(prescaler))
		return -1;

	if (SPI1 == SPIx)
		spi_init = &SPI_InitStructure[0];
	else if (SPI2 == SPIx) {
		spi_init = &SPI_InitStructure[1];
	} else if (SPI3 == SPIx) {
		spi_init = &SPI_InitStructure[2];
	} else {
		spi_init = 0;
	}
	//异常不处理
	if (0 != spi_init) {

		//禁用SPI2外设
		SPI_Cmd(SPIx, DISABLE);

		//修改速率
		spi_init->SPI_BaudRatePrescaler = prescaler; //SPI波特率预分频值为8
		SPI_Init(SPIx, spi_init); //根据SPI_InitStruct中指定的参数初始化外设SPI2寄存器

		//使能SPI2外设
		SPI_Cmd(SPIx, ENABLE);
		return 0; //成功
	}

	return -1;
}
//直接修改波特率寄存器的方式控制SPI的速度
void SPIx_MasterSetCR(SPI_TypeDef* SPIx,uint16_t cr)
{
	SPIx->CR1=cr;
}
uint16_t SPIx_MasterGetCR(SPI_TypeDef* SPIx)
{
	return SPIx->CR1;
}
