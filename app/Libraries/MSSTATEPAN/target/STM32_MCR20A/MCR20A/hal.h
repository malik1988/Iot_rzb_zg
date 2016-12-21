/*
  V0.1 Initial Release   10/July/2006  RBR

*/

/*
Original hal.h file from Chipcon for CC2430 modified to include
only the functions needed by this stack and test code

*/



/******************************************************************************
*                                                                             *
*        **********                                                           *
*       ************                                                          *
*      ***        ***                                                         *
*     ***    ++    ***                                                        *
*     ***   +  +   ***                      CHIPCON                           *
*     ***   +                                                                 *
*     ***   +  +   ***                                                        *
*     ***    ++    ***                                                        *
*      ***        ***                                                         *
*       ************                                                          *
*        **********                                                           *
*                                                                             *
*******************************************************************************

Filename:     hal.h
Target:       MCR20AVHM
Author:       LiXianmao
Revised:      2016.9.22
Revision:     1.0

Description:
Hardware Abstraction Layer - Utility Library for MCR20AVHM.

******************************************************************************/

#ifndef HAL_H
#define HAL_H
#include <stdio.h>
#include <string.h>
#include "compiler.h"
#include "lrwpan_config.h"         //user configurations
#include "lrwpan_common_types.h"   //types common acrosss most files


//-----------------------------------------------------------------------------
// Common values
#ifndef FALSE
   #define FALSE 0
#endif

#ifndef TRUE
   #define TRUE 1
#endif

#ifndef NULL
   #define NULL 0
#endif

#ifndef HIGH
   #define HIGH 1
#endif

#ifndef LOW
   #define LOW 0
#endif





#define HAL_SUSPEND(x)         //dummy in uC, only needed for Win32

//Timer Support
//For the CC2430, we set the timer to exactly 1 tick per symbol
//assuming a clock frequency of 32MHZ, and 2.4GHz
#define SYMBOLS_PER_MAC_TICK()     1
//
#define SYMBOLS_TO_MACTICKS(x) (x/SYMBOLS_PER_MAC_TICK())   
#if MACTICK_USE_SYSTICK
//每一个symbol对应于16us，x为ms数，计算出所需要的ticks数
//一秒的ticks=x*LRWPAN_SYMBOLS_PER_SECOND，转换为ms=x*LRWPAN_SYMBOLS_PER_SECOND/1000
#define MSECS_TO_MACTICKS(x)   (x*(LRWPAN_SYMBOLS_PER_SECOND/1000))
#else
#define MSECS_TO_MACTICKS(x)  	(x)
#endif

#define MACTIMER_MAX_VALUE 0xFFFFFFFF   //32 bit counter
#define halMACTimerNowDelta(x) ((halGetMACTimer()-(x))& MACTIMER_MAX_VALUE)
#define halMACTimerDelta(x,y) ((x-(y))& MACTIMER_MAX_VALUE)

/******************************************************************************
*******************       Interrupt functions/macros        *******************
******************************************************************************/

// Macros which simplify access to interrupt enables, interrupt flags and
// interrupt priorities. Increases code legibility.

//******************************************************************************





/******************************************************************************
*******************         Common USART functions/macros   *******************
******************************************************************************/


/*
#### RADIO Support
*/

#define STOP_RADIO()        MCR20A_Abort()


//设备角色，协调器或者终端
#define PAN_COORDINATOR     1
#define PAN_ENDDIVCE		0


//保存中断寄存器并关闭中断
#define SAVE_AND_DISABLE_GLOBAL_INTERRUPT(x) 
//还原中断寄存器
#define RESTORE_GLOBAL_INTERRUPT(x) 


//#ifndef evbPoll
//#define evbPoll() 
//#endif


//ZigBee标准 帧数据最后两字节为FCS(CRC)
#define PACKET_FOOTER_SIZE 2    //bytes after the payload


/******************************************************************************
*******************              Utility functions          *******************
******************************************************************************/

/******************************************************************************
* @fn  halWait
*
* @brief
*      This function waits approximately a given number of m-seconds
*      regardless of main clock speed.
*
* Parameters:
*
* @param  BYTE	 wait
*         The number of m-seconds to wait.
*
* @return void
*
******************************************************************************/
void halWait(BYTE wait);





#endif //HAL_H
