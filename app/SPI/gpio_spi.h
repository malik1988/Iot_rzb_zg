#ifndef GPIO_SPI_H
#define GPIO_SPI_H

#include "spi.h"	//直接引用头文件中的管脚定义
#define SCK_OUT_HIGH()	   GPIO_SetBits(SPI1_SCK_PORT,SPI1_SCK)
#define SCK_OUT_LOW()	   GPIO_ResetBits(SPI1_SCK_PORT,SPI1_SCK)

#define MOSI_OUT_HIGH()	   GPIO_SetBits(SPI1_MOSI_PORT,SPI1_MOSI)
#define MOSI_OUT_LOW()	   GPIO_ResetBits(SPI1_MOSI_PORT,SPI1_MOSI)

#define MISO_IN_HIGH()		GPIO_ReadInputDataBit(SPI1_MISO_PORT,SPI1_MISO)
#define MISO_IN_LOW()		GPIO_ReadInputDataBit(SPI1_MISO_PORT,SPI1_MISO)

#define CPOL  			0
#define CPHA			0
#define MSB_FIRST		1	//目前只支持MSB_FIRST=1


void SPI1_GPIO_Init();
void SPI1_GPIO_SendByte(uint8_t data);
uint8_t SPI1_GPIO_ReadByte();
uint8_t SPI1_GPIO_WriteRead(uint8_t data);

#endif //ENDOF GPIO_SPI_H
