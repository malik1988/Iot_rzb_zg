#include "gpio_spi.h"

//等待一段 时间保证数据稳定
void SPI_GPIO_WAIT() {
	uint8_t i, j;
	for (i = 0; i < 5; i++)
		for (j = 0; j < 100; j++)
			;
}

//初始化GPIO管脚
void SPI1_GPIO_IO_Init() {
	GPIO_InitTypeDef GPIO_InitStructure;

	//配置AHB_ENABLE
	SPI1_AHB_EN();

	//配置PIN
	//PA5/PA6
	GPIO_InitStructure.GPIO_Pin = SPI1_SCK | SPI1_MISO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//PB5
	GPIO_InitStructure.GPIO_Pin = SPI1_MOSI;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}

//初始化SPI1，设置SCK、MOSI、CS、MISO初始值
void SPI1_GPIO_Init() {
	SPI1_GPIO_IO_Init();
	SCK_OUT_LOW();
	SPI1_GPIO_WAIT();

}

void SPI1_GPIO_SendByte(uint8_t data) {
	uint8_t i;
	for (i = 0; i < 8; i++) {
		SCK_OUT_LOW();
		SPI1_GPIO_WAIT();

#if MSB_FIRST
		if (data & 0x80)
			MOSI_OUT_HIGH();
		else
			MOSI_OUT_LOW();
		data <<= 1;
#else
		if(data&0x01)
		MOSI_OUT_HIGH();
		else
		MOSI_OUT_LOW();
		data>>=1;
#endif

		SCK_OUT_HIGH();
		SPI1_GPIO_WAIT();
	}

	SCK_OUT_LOW();
	SPI1_GPIO_WAIT();
}

uint8_t SPI1_GPIO_ReadByte() {
	uint8_t i, data = 0;
	for (i = 0; i < 8; i++) {
		SCK_OUT_LOW();
		SPI1_GPIO_WAIT();
#if MSB_FIRST
		data <<= 1; //下一个周期进来时上个周期的数据在前
#endif
		if (MISO_IN_HIGH())
			data |= 0x01;
		else
			data &= 0xFE;

		SCK_OUT_HIGH();
		SPI1_GPIO_WAIT();
	}

	SCK_OUT_LOW();
	SPI1_GPIO_WAIT();
	return data;
}

uint8_t SPI1_GPIO_WriteRead(uint8_t data) {
	uint8_t i, ret = 0;
	for (i = 0; i < 8; i++) {
		SCK_OUT_LOW();
		SPI1_GPIO_WAIT();

		if (data & 0x80)
			MOSI_OUT_HIGH();
		else
			MOSI_OUT_LOW();
		data <<= 1;

		ret <<= 1;
		if (MISO_IN_HIGH())
			ret |= 0x01;
		else
			ret &= 0xFE;

		SCK_OUT_HIGH();
		SPI1_GPIO_WAIT();
	}

	SCK_OUT_LOW();
	SPI1_GPIO_WAIT();

	return ret;
}
