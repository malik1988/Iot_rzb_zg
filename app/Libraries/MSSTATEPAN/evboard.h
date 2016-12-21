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






#ifndef EVBOARD_H
#define EVBOARD_H


#include "board.h"

typedef union _EVB_SW_STATE {
  BYTE val;
  struct _SW_STATE_bits {
    unsigned s1_val:1;
    unsigned s1_last_val:1;
    unsigned s1_tgl:1;
    unsigned s2_val:1;
    unsigned s2_last_val:1;
    unsigned s2_tgl:1;
  }bits;
}EVB_SW_STATE;



extern EVB_SW_STATE sw_state;


#define EVB_LED1_ON()      Board_LedOn(ALARM)
#define EVB_LED1_OFF()     Board_LedOff(ALARM)
#define EVB_LED2_ON()      Board_LedOn(RUN)
#define EVB_LED2_OFF()     Board_LedOff(RUN)

#define EVB_LED1_STATE()  1 //evbLedGet(1)
#define EVB_LED2_STATE()   1 //evbLedGet(2)


#define EVB_SW1_PRESSED()     (sw_state.bits.s1_val)
#define EVB_SW1_RELEASED()    (!sw_state.bits.s1_val)
#define EVB_SW2_PRESSED()     (sw_state.bits.s2_val)
#define EVB_SW2_RELEASED()    (!sw_state.bits.s2_val)


#define EVB_SW1_TOGGLED()     (sw_state.bits.s1_tgl)
#define EVB_SW1_CLRTGL()      sw_state.bits.s1_tgl=0
#define EVB_SW2_TOGGLED()     (sw_state.bits.s2_tgl)
#define EVB_SW2_CLRTGL()      sw_state.bits.s2_tgl=0



//void evbLedSet(BYTE lednum, BOOL state);
//BOOL evbLedGet(BYTE lednum);

//prototypes for bind resolution, bind table support
//need to add some functions for adding bindings

BOOL evbBindTableIterInit(BYTE srcEP, SADDR srcSADDR, BYTE cluster);
BOOL evbResolveBind(BYTE *dstEP, SADDR *dstSADDR);

#endif




