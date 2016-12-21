//神舟开发板
#include "NRF24l01.h"
#include "spi.h"

#ifndef SPI2_ReadWriteByte
#define SPI2_ReadWriteByte(x) SPIx_WriteReadByte(SPI2,x)
#endif

//NRF24L01 驱动函数 

//TX_ADDR 发送端的地址
const u8 TX_ADDRESS[TX_ADR_WIDTH] = { 0x34, 0x43, 0x10, 0x10, 0x01 };
//RX_ADDR 接收端的地址
const u8 RX_ADDRESS[RX_ADR_WIDTH] = { 0x34, 0x43, 0x10, 0x10, 0x01 };
//初始化24L01的IO口
void NRF24L01_Init(void) {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_NRF24L01_CE, ENABLE); //使能GPIO的时钟
	GPIO_InitStructure.GPIO_Pin = NRF24L01_CE; //NRF24L01 模块片选信号
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIO_NRF24L01_CE, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd(RCC_NRF24L01_CSN, ENABLE); //使能GPIO的时钟
	GPIO_InitStructure.GPIO_Pin = NRF24L01_CSN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIO_NRF24L01_CSN, &GPIO_InitStructure);

	Set_NRF24L01_CE(); //初始化时先拉高
	Set_NRF24L01_CSN(); //初始化时先拉高

	//配置NRF2401的IRQ
	RCC_AHB1PeriphClockCmd(RCC_NRF24L01_IRQ, ENABLE);
	GPIO_InitStructure.GPIO_Pin = NRF24L01_IRQ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; //上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIO_NRF24L01_IRQ, &GPIO_InitStructure);

	SPI2_Init(); //初始化SPI
	Clr_NRF24L01_CE(); //使能24L01
	Set_NRF24L01_CSN(); //SPI片选取消
}
//上电检测NRF24L01是否在位
//写5个数据然后再读回来进行比较，相同时返回值:0，表示在位;否则返回1，表示不在位	
u8 NRF24L01_Check(void) {
	u8 buf[5] = { 0XA5, 0XA5, 0XA5, 0XA5, 0XA5 };
	u8 buf1[5];
	u8 i;
	NRF24L01_Write_Buf(SPI_WRITE_REG + TX_ADDR, buf, 5); //写入5个字节的地址.
	NRF24L01_Read_Buf(TX_ADDR, buf1, 5); //读出写入的地址
	for (i = 0; i < 5; i++) {
		if (buf1[i] != 0XA5) { //printf("%X ",buf1[i]);
			break;
		}
	}
	if (i != 5)
		return 1; //NRF24L01不在位
	return 0; //NRF24L01在位
}
//通过SPI写寄存器
u8 NRF24L01_Write_Reg(u8 regaddr, u8 data) {
	u8 status;
	Clr_NRF24L01_CSN(); //使能SPI传输
	status = SPI2_ReadWriteByte(regaddr); //发送寄存器号
	SPI2_ReadWriteByte(data);
	//写入寄存器的值
	Set_NRF24L01_CSN(); //禁止SPI传输
	return (status); //返回状态值
}
//读取SPI寄存器值 ，regaddr:要读的寄存器
u8 NRF24L01_Read_Reg(u8 regaddr) {
	u8 reg_val;
	Clr_NRF24L01_CSN(); //使能SPI传输
	SPI2_ReadWriteByte(regaddr);
	//发送寄存器号
	reg_val = SPI2_ReadWriteByte(0XFF); //读取寄存器内容
	Set_NRF24L01_CSN(); //禁止SPI传输
	return (reg_val); //返回状态值
}
//在指定位置读出指定长度的数据
//*pBuf:数据指针
//返回值,此次读到的状态寄存器值 
u8 NRF24L01_Read_Buf(u8 regaddr, u8 *pBuf, u8 datalen) {
	u8 status, u8_ctr;
	Clr_NRF24L01_CSN(); //使能SPI传输
	status = SPI2_ReadWriteByte(regaddr); //发送寄存器值(位置),并读取状态值
	for (u8_ctr = 0; u8_ctr < datalen; u8_ctr++)
		pBuf[u8_ctr] = SPI2_ReadWriteByte(0XFF); //读出数据
	Set_NRF24L01_CSN(); //关闭SPI传输
	return status; //返回读到的状态值
}
//在指定位置写指定长度的数据
//*pBuf:数据指针
//返回值,此次读到的状态寄存器值
u8 NRF24L01_Write_Buf(u8 regaddr, u8 *pBuf, u8 datalen) {
	u8 status, u8_ctr;
	Clr_NRF24L01_CSN(); //使能SPI传输
	status = SPI2_ReadWriteByte(regaddr); //发送寄存器值(位置),并读取状态值
	for (u8_ctr = 0; u8_ctr < datalen; u8_ctr++)
		SPI2_ReadWriteByte(*pBuf++);
	//写入数据
	Set_NRF24L01_CSN(); //关闭SPI传输
	return status; //返回读到的状态值
}
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:发送完成状况
u8 NRF24L01_TxPacket(u8 *txbuf) {
	u8 state;
	u32 timeout = 0;
	Clr_NRF24L01_CE();
	NRF24L01_Write_Buf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH); //写数据到TX BUF  32个字节
	Set_NRF24L01_CE(); //启动发送
	while (READ_NRF24L01_IRQ != 0) {
		timeout++;
		if (timeout > 3000)
			break;
	}; //等待发送完成
	state = NRF24L01_Read_Reg(STATUS); //读取状态寄存器的值
	NRF24L01_Write_Reg(SPI_WRITE_REG + STATUS, state); //清除TX_DS或MAX_RT中断标志
	if (state & MAX_TX) //达到最大重发次数
	{
		NRF24L01_Write_Reg(FLUSH_TX, 0xff); //清除TX FIFO寄存器
		return MAX_TX;
	}
	if (state & TX_OK) //发送完成
	{
		return TX_OK;
	}
	return 0xff; //其他原因发送失败
}
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:0，接收完成；其他，错误代码
u8 NRF24L01_RxPacket(u8 *rxbuf) {
	u8 state;
	state = NRF24L01_Read_Reg(STATUS); //读取状态寄存器的值
	//printf("\n\r Rx state %X", state);
	NRF24L01_Write_Reg(SPI_WRITE_REG + STATUS, state); //清除TX_DS或MAX_RT中断标志
	if (state & RX_OK) //接收到数据
	{
		NRF24L01_Read_Buf(RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH); //读取数据
		NRF24L01_Write_Reg(FLUSH_RX, 0xff); //清除RX FIFO寄存器
		return 0;
	}
	return 1; //没收到任何数据
}

//该函数初始化NRF24L01到RX模式
//设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR
//当CE变高后,即进入RX模式,并可以接收数据了		   
void RX_Mode(void) {
	Clr_NRF24L01_CE();
	NRF24L01_Write_Buf(SPI_WRITE_REG + RX_ADDR_P0, (u8*) RX_ADDRESS,
			RX_ADR_WIDTH); //写RX节点地址
	NRF24L01_Write_Buf(SPI_WRITE_REG + TX_ADDR, (u8*) TX_ADDRESS, TX_ADR_WIDTH); //写TX节点地址

	NRF24L01_Write_Reg(SPI_WRITE_REG + EN_AA, 0x01); //使能通道0的自动应答
	NRF24L01_Write_Reg(SPI_WRITE_REG + EN_RXADDR, 0x01); //使能通道0的接收地址
	NRF24L01_Write_Reg(SPI_WRITE_REG + RF_CH, RF_CHANNEL); //设置RF通信频率
	NRF24L01_Write_Reg(SPI_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH); //选择通道0的有效数据宽度
	NRF24L01_Write_Reg(SPI_WRITE_REG + RF_SETUP, 0x0f); //设置TX发射参数,0db增益,2Mbps,低噪声增益开启
	NRF24L01_Write_Reg(SPI_WRITE_REG + CONFIG, 0x0f); //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式
	Set_NRF24L01_CE(); //CE为高,进入接收模式
}
//该函数初始化NRF24L01到TX模式
//设置TX地址,写TX数据宽度,设置RX自动应答的地址,填充TX发送数据,选择RF频道,波特率和LNA HCURR
//PWR_UP,CRC使能
//当CE变高后,即进入RX模式,并可以接收数据了		   
//CE为高大于10us,则启动发送.	 
void TX_Mode(void) {
	Clr_NRF24L01_CE();
	NRF24L01_Write_Buf(SPI_WRITE_REG + TX_ADDR, (u8*) TX_ADDRESS, TX_ADR_WIDTH); //写TX节点地址
	NRF24L01_Write_Buf(SPI_WRITE_REG + RX_ADDR_P0, (u8*) RX_ADDRESS,
			RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK

	NRF24L01_Write_Reg(SPI_WRITE_REG + EN_AA, 0x01); //使能通道0的自动应答
	NRF24L01_Write_Reg(SPI_WRITE_REG + EN_RXADDR, 0x01); //使能通道0的接收地址
	NRF24L01_Write_Reg(SPI_WRITE_REG + SETUP_RETR, 0x1a); //设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
	NRF24L01_Write_Reg(SPI_WRITE_REG + RF_CH, RF_CHANNEL); //设置RF通道为0x01
	NRF24L01_Write_Reg(SPI_WRITE_REG + RF_SETUP, 0x0f); //设置TX发射参数,0db增益,2Mbps,低噪声增益开启
	NRF24L01_Write_Reg(SPI_WRITE_REG + CONFIG, 0x0e); //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
	Set_NRF24L01_CE(); //CE为高,10us后启动发送

	printf("Addr %d\n",TX_ADDRESS[4]);

}
//调试使用
void Read_Mode() {
	uint8_t reg;
	reg = NRF24L01_Read_Reg(EN_AA);
	reg = NRF24L01_Read_Reg(EN_RXADDR);
	reg = NRF24L01_Read_Reg(SETUP_RETR);
	reg = NRF24L01_Read_Reg(RF_CH);
	reg = NRF24L01_Read_Reg(RF_SETUP);
	reg = NRF24L01_Read_Reg(CONFIG);
}

void RX_Mode_Addr(u8 * addr) {
	Clr_NRF24L01_CE();
	NRF24L01_Write_Buf(SPI_WRITE_REG + RX_ADDR_P0, (u8*) addr,
			RX_ADR_WIDTH); //写RX节点地址
	NRF24L01_Write_Buf(SPI_WRITE_REG + TX_ADDR, (u8*) addr, TX_ADR_WIDTH); //写TX节点地址
	Set_NRF24L01_CE(); //CE为高,进入接收模式
}
