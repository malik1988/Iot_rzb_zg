/*
 * IAP在线编程，在线升级程序
 * */

#include "board.h"
#if BOOT_LOADER
#include "iap.h"
#include "xprintf.h"


#define FILE_LEN_VER		4		//版本号字节数

#if BOOT_MD5_CHECK
#include "md5.h"
#endif

#define IAP_PROGRAM_LEN		1024*32		//应用程序的总大小
#define IAP_BUF_MAX_LEN		1024*100	//IAP缓冲最大空间,100K
uint8_t iap_buf[IAP_BUF_MAX_LEN]; //定义数据空间，不能超出RAM大小，STM32F207VE RAM 128K
uint8_t iap_file_end;//获取到了完整 的文件，文件获取结束

#define IAP_FLASH_SECTOR 			FLASH_Sector_5		//IAP FLASH块编号
#define IAP_FLASH_BASE				FLASH_BASE_SECTOR_5	//IAP FLASH首地址
typedef void (*pFunction)(void);

//检查文件是否是当前板子的，是否匹配当前板子
uint8_t IAP_Board_Match(uint32_t id) {
#define FILE_ID_RZB 	(0X11<<24)
#define FILE_ID_ZG		(0x22<<24)
#define FILE_ID_MASK	(0XFF<<24)

	uint8_t ret = 0;
	if (Board_Is_RZB()) { //当前板子是RZB
		if (id & FILE_ID_RZB) {
			ret = 1;
		}
	} else { //当前板子是ZG
		if (id & FILE_ID_ZG) {
			ret = 1;
		}
	}
	return ret;
}

#if BOOT_MD5_CHECK

#define	FILE_LEN_MD5		16		//MD5占用字节数
//对接收到的数据进行MD5校验，返回值1--校验成功，0--校验失败
//len--IAP缓冲中有效数据的长度，iap_buf[len]开始为MD5的值
uint8_t IAP_MD5_Match(uint32_t len)
{
	uint8_t hash[FILE_LEN_MD5];
	md5(hash, (const uint8_t*)iap_buf, len);
	for(uint8_t i=0;i<sizeof(hash);i++)
	{
		if(hash[i]!=iap_buf[len+i]) {
			return 0; //校验失败
		}
	}
	Dprintf("MD5 校验成功！\n");
	return 1;//校验成功
}
#endif

//IAP服务程序，启动后接收程序文件，烧写入FLASH并自动重启,len--期望的文件大小，需要手动输入，用于校验文件是否正常
//返回值，1--表示IAP获取并烧写成功，0--表示IAP获取文件或烧写失败，需要重新尝试
uint8_t IAP_Service(uint32_t len) {
	uint32_t file_len = 0;
	uint32_t *ptr; //写入Flash指针
	pFunction pf;//
	//uint8_t ret = 0;//返回值

	iap_file_end = 0;//初始化标志位为0

	uint8_t buf1k[1024];//1K缓冲区
	file_len = DbgReceive1KXModem(buf1k, iap_buf, sizeof(iap_buf));
	delayMS(200);//延迟200ms
	if (file_len > 0 && file_len < (IAP_BUF_MAX_LEN)) {


		//检查接收到的文件是否是一个有效的应用程序
		if (IAP_Program_Check_Buf()) {
			//寻找0x1A1A,xmodem结束符，并过滤掉
			uint16_t *xPtr = iap_buf + file_len - 2;
#define XMODEM_END_U16	0x1a1a
			while (*xPtr == XMODEM_END_U16) {
				xPtr--;
				file_len -= 2; //文件长度减去结束符长度（2字节）
			}

#if BOOT_MD5_CHECK

			if(IAP_MD5_Match(file_len-FILE_LEN_MD5-FILE_LEN_VER)==0)
			{//校验失败
				printf("\n无效文件！\n");
				return 0;
			}
#endif

			uint32_t *verPtr;
			verPtr = iap_buf + file_len - FILE_LEN_VER; //最后4字节为版本号
			if (IAP_Board_Match(*verPtr) && Board_Version_Check(*verPtr)) { //文件是否匹配当前硬件，且版本号合法

				printf("\n获取到文件长度：%d,版本号: ", file_len);
				Board_Version_Print((*verPtr) & BOARD_VERSION_MASK);
				Device_Info_t *info = Board_DeviceInfo_Get();
				//检查板上软件版本号是否合法，不合法表示没设置过，忽略版本比较
				if (Board_Version_Check(info->ver_sw)) {

					//获取的文件版本号必须>=板上固有的软件版本号，否则不会升级
					if ((*verPtr) >= (info->ver_sw)) {
						//文件有效
						iap_file_end = 1;
					}

				} else { //当前板上版本号缺失，忽略版本号比较
					//文件有效
					iap_file_end = 1;
				}

			} else { //IAP_Program_Check_Buf 检验失败
				printf("\n无效文件！\n");
				return 0;
			}

		} else {
			printf("\n无效文件！\n");
			return 0;
		}

	} //ENDOF if (file_len > 0 && file_len < (IAP_BUF_MAX_LEN))

	if (iap_file_end) {
		//开始写入Flash
		FLASH_Unlock();
		FLASH_ClearFlag(
				FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR
				| FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR
				| FLASH_FLAG_PGSERR);
		//擦除整个数据块
		while (FLASH_EraseSector(IAP_FLASH_SECTOR, VoltageRange_3)
				!= FLASH_COMPLETE)
		;

		//获取待写入的数据首地址
		ptr = (uint32_t) iap_buf;

		for (uint32_t i = 0; i < (file_len / sizeof(uint32_t)); i++) {
			FLASH_ProgramWord(IAP_FLASH_BASE + i * 4, ptr[i]);
		}
		//写入完成
		FLASH_Lock();
		return 1;
	}
	return 0;
}
//从IAP_BUF中检查是否存在应用程序
uint8_t IAP_Program_Check_Buf() {
	uint32_t *ptr = (uint32_t*) iap_buf;
	if (((*(__IO uint32_t*) ptr) & 0x2FFE0000) == 0x20020000) {
		return 1;
	}
	return 0;
}

//检查IAP FLASH区域的程序是否存在
uint8_t IAP_Program_Check() {
	//因为用户程序开始位置(0x08008000处)的前4个字节存放的是堆栈的地址，
	//堆栈地址必定是指向RAM空间的，而STM32的RAM空间起始地址为0x20020000。
	if (((*(__IO uint32_t*) IAP_FLASH_BASE) & 0x2FFE0000) == 0x20020000) {
		return 1;
	}
	return 0;
}

void IAP_Program_Erase() {
	FLASH_Unlock();
	FLASH_ClearFlag(
			FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR
			| FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR
			| FLASH_FLAG_PGSERR);
	//擦除整个数据块
	while (FLASH_EraseSector(IAP_FLASH_SECTOR, VoltageRange_3) != FLASH_COMPLETE)
	;

	FLASH_Lock();
}
//运行用户程序
void IAP_Program_Run() {
	pFunction proPtr; //复位程序指针

	//将复位程序中断向量指向应用程序地址
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, IAP_FLASH_BASE - FLASH_BASE);
	//IAP_FLASH_BASE+4 指向的是复位程序地址
	proPtr = *(__IO uint32_t*) (IAP_FLASH_BASE + 4);

	//堆栈的初始化，重新设定栈顶代地址，把栈顶地址设置为用户代码指向的栈顶地址。
	__set_MSP(*(__IO uint32_t*) IAP_FLASH_BASE);
	//执行复位程序，系统将复位后执行用户程序
	proPtr();

}

#endif //EDNOF BOOT_LOADER
