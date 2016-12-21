#ifndef SPI_H
#define SPI_H

#include "board.h"

//管脚
#define GPIO_PIN(x)			 (GPIO_Pin_##x)
#define GPIO_PinSource(x) (GPIO_PinSource##x)


/*---------------SPI1--只初始化SCK,MISO,MOSI三线，CS需要单独配置-------------*/

#define SPI1_AHB_EN()  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	\
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE)
#define SPI1_CLK_EN()   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE)		//开启CLK
#define SPI1_CLK_DEN()   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE)	//禁用CLK


#if RZG
#define SPI1_SCK                    GPIO_Pin_5	//PA5
#define SPI1_MISO                   GPIO_Pin_6	//PA6
#define SPI1_MOSI                   GPIO_Pin_5	//PB5
#define SPI1_SCK_SOURCE             GPIO_PinSource5
#define SPI1_MISO_SOURCE            GPIO_PinSource6
#define SPI1_MOSI_SOURCE            GPIO_PinSource5

#define SPI1_SCK_PORT				GPIOA
#define SPI1_MISO_PORT				GPIOA
#define SPI1_MOSI_PORT				GPIOB

#elif RZB
#define SPI1_SCK                    GPIO_Pin_5	//PA5
#define SPI1_MISO                   GPIO_Pin_6	//PA6
#define SPI1_MOSI                   GPIO_Pin_7	//PA7
#define SPI1_SCK_SOURCE             GPIO_PinSource5
#define SPI1_MISO_SOURCE            GPIO_PinSource6
#define SPI1_MOSI_SOURCE            GPIO_PinSource7

#define SPI1_SCK_PORT				GPIOA
#define SPI1_MISO_PORT				GPIOA
#define SPI1_MOSI_PORT				GPIOA

#else	//神舟开发板

#define SPI1_SCK                    GPIO_Pin_3	//PB3
#define SPI1_MISO                   GPIO_Pin_4	//PB4
#define SPI1_MOSI                   GPIO_Pin_5	//PB5
#define SPI1_SCK_SOURCE             GPIO_PinSource3
#define SPI1_MISO_SOURCE            GPIO_PinSource4
#define SPI1_MOSI_SOURCE            GPIO_PinSource5

#define SPI1_SCK_PORT				GPIOB
#define SPI1_MISO_PORT				GPIOB
#define SPI1_MOSI_PORT				GPIOB

#endif



/*---------------SPI2--只初始化SCK,MISO,MOSI三线，CS需要单独配置-------------*/

#define SPI2_AHB_EN()  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE)
#define SPI2_CLK_EN()   RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE)		//开启CLK
#define SPI2_CLK_DEN()   RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, DISABLE)	//禁用CLK
#define SPI2_GPIO_PORT              GPIOB
#define SPI2_SCK                    GPIO_Pin_13
#define SPI2_MISO                   GPIO_Pin_14
#define SPI2_MOSI                   GPIO_Pin_15
#define SPI2_SCK_SOURCE             GPIO_PinSource13
#define SPI2_MISO_SOURCE            GPIO_PinSource14
#define SPI2_MOSI_SOURCE            GPIO_PinSource15





void SPI1_Init(void);
void SPI2_Init(void);
u8 SPIx_WriteReadByte(SPI_TypeDef* SPIx,u8 TxData);
u8 SPIx_Read(SPI_TypeDef* SPIx);
void SPIx_PowerOn(SPI_TypeDef* SPIx);
void SPIx_PowerOff(SPI_TypeDef* SPIx);
int SPIx_MasterSpeed(SPI_TypeDef* SPIx, uint16_t prescaler);
void SPIx_MasterSetCR(SPI_TypeDef* SPIx,uint16_t cr);
uint16_t SPIx_MasterGetCR(SPI_TypeDef* SPIx);

#endif //endof SPI_H

