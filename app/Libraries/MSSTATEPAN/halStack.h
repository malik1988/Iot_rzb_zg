/*
 * "Copyright (c) 2006 Robert B. Reese ("AUTHOR")"
 * All rights reserved.
 * (R. Reese, reese@ece.msstate.edu, Mississippi State University)
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice, the following
 * two paragraphs and the author appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE "AUTHOR" BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE "AUTHOR"
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE "AUTHOR" SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE "AUTHOR" HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Please maintain this header in its entirety when copying/modifying
 * these files.
 *
 * Files in this software distribution may have different usage
 * permissions, see the header in each file. Some files have NO permission
 * headers since parts of the original sources in these files
 * came from vendor  sources with no usage restrictions.
 *
 */

/*
V0.2 added PC-based binding         21/July/2006  RBR
V0.1 Initial Release                10/July/2006  RBR

*/



//HAL functions need by Stack



#ifndef HALSTACK_H
#define HALSTACK_H

#include "MCR20A_Phy.h"
#include "compiler.h"
#include "lrwpan_common_types.h"

//these are prototypes for HAL functions called from the stack
//if a HAL function is not in here, then it is in the
//hal.h file in the target subdirectory


void halInit(void);  //processor, board specific initializations

void halInitUart(void);  // do everything except baud rate setting.

char halGetch(void);  //get a character from serial port
BOOL halGetchRdy(void);  //is a character available from the serial port?
void halPutch(char c);  //write a character to serial port
void halRawPut(char c);  //write a byte to serial port, no character interpretation
void halInitMACTimer(void); //init timer used for Radio timeouts
UINT32 halGetMACTimer(void); //return timer value
#ifdef LRWPAN_COMPILER_NO_RECURSION		//编译是否允许递归调用
UINT32 halISRGetMACTimer(void); //return timer value
#else
#define halISRGetMACTimer() halGetMACTimer()
#endif


LRWPAN_STATUS_ENUM halInitRadio(PHY_FREQ_ENUM frequency, BYTE channel, RADIO_FLAGS radio_flags);
void halGetProcessorIEEEAddress(BYTE *buf);
void halSetRadioIEEEAddress(void );
LRWPAN_STATUS_ENUM halSetRadioIEEEFrequency(PHY_FREQ_ENUM frequency, BYTE channel);
void halSetRadioPANID(UINT16 panid);
void halSetRadioShortAddr(SADDR saddr);
LRWPAN_STATUS_ENUM halSendPacket(BYTE flen, BYTE *frm);
LRWPAN_STATUS_ENUM halSetChannel(BYTE channel);
UINT32 halMacTicksToUs(UINT32 x);

//用户自定义扩展函数接口
UINT8 halDataEncryptAndDecrypt(UINT8 key[16], UINT8 in[16],UINT8 cnt[16], UINT8 out[16]);



UINT8 halGetRandomByte(void);
void halSleep(UINT32 msecs);    //put processor to sleep
void halSuspend(UINT32 msecs);  //suspends process, intended for Win32, dummy on others
void halUtilMemCopy(BYTE *dst, BYTE *src, BYTE len);
void halWaitMs(UINT32 msecs);
void halShutdown(void);
void halWarmstart(void);


//call backs to PHY, MAC from HAL
void phyRxCallback(void);
void phyTxStartCallBack(void);
void phyTxEndCallBack(void);
void macRxCallback(BYTE *ptr, BYTE rssi);
void macTxCallback(void);

void evbIntCallback(void);  //Evaluation board slow timer interrupt callback
void usrIntCallback(void);   //general interrupt callback , when this is called depends on the HAL layer.
#ifdef LRWPAN_ENABLE_SLOW_TIMER
void usrSlowTimerInt(void); //user interrupt slow timer interrupt callback
#endif


#endif

