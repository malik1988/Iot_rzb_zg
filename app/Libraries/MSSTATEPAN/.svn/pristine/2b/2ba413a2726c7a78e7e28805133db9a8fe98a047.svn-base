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


#ifndef LRWPAN_COMMON_TYPES_H
#define LRWPAN_COMMON_TYPES_H

#include ".\target\STM32_MCR20A\MCR20A\compiler.h"   //



//types common across stack or multiple stack layers

//common macros
#define BITSET(var,bitno) ((var) |= (1 << (bitno)))
#define BITCLR(var,bitno) ((var) &= ~(1 << (bitno)))
#define BITTST(var,bitno) (var & (1 << (bitno)))



typedef enum _LRWPAN_SVC_ENUM {
  LRWPAN_SVC_NONE,
  LRWPAN_SVC_PHY_INIT_RADIO,
  LRWPAN_SVC_PHY_TX_DATA,
  LRWPAN_SVC_MAC_GENERIC_TX,
  LRWPAN_SVC_MAC_RETRANSMIT,
  LRWPAN_SVC_MAC_ASSOC_REQ,
  LRWPAN_SVC_MAC_BEACON_REQ,
  LRWPAN_SVC_MAC_ORPHAN_NOTIFY,
  LRWPAN_SVC_MAC_ERROR,
  LRWPAN_SVC_NWK_GENERIC_TX,
  LRWPAN_SVC_NWK_FORM_NETWORK,
  LRWPAN_SVC_NWK_JOIN_NETWORK,
  LRWPAN_SVC_APS_GENERIC_TX,
  LRWPAN_SVC_APS_NWK_PASSTHRU,
  LRWPAN_SVC_APS_DO_ZEP_TX,
} LRWPAN_SVC_ENUM;



typedef enum _LRWPAN_STATUSENUM {
  LRWPAN_STATUS_SUCCESS = 0,
  LRWPAN_STATUS_PHY_FAILED,
  LRWPAN_STATUS_PHY_INPROGRESS,  //still working for splitphase operations
  LRWPAN_STATUS_PHY_RADIO_INIT_FAILED,
  LRWPAN_STATUS_PHY_TX_PKT_TOO_BIG,
  LRWPAN_STATUS_PHY_TX_START_FAILED,
  LRWPAN_STATUS_PHY_TX_FINISH_FAILED,
  LRWPAN_STATUS_PHY_CHANNEL_BUSY,
  LRWPAN_STATUS_MAC_FAILED,
  LRWPAN_STATUS_MAC_NOT_ASSOCIATED,
  LRWPAN_STATUS_MAC_INPROGRESS,  //still working for splitphase operations
  LRWPAN_STATUS_MAC_MAX_RETRIES_EXCEEDED,  //exceeded max retries
  LRWPAN_STATUS_MAC_TX_FAILED,    //MAC Tx Failed, retry count exceeded
  LRWPAN_STATUS_MAC_ASSOCIATION_TIMEOUT,  //association request timedout
  LRWPAN_STATUS_MAC_ORPHAN_TIMEOUT,       //ophan notify timedout
  LRWPAN_STATUS_NWK_INPROGRESS,
  LRWPAN_STATUS_NWK_JOIN_TIMEOUT,
  LRWPAN_STATUS_NWK_PACKET_UNROUTABLE,
  LRWPAN_STATUS_NWK_RADIUS_EXCEEDED,
  LRWPAN_STATUS_APS_INPROGRESS,
  LRWPAN_STATUS_APS_MAX_RETRIES_EXCEEDED,
  LRWPAN_STATUS_APS_ILLEGAL_ENDPOINT,
  LRWPAN_STATUS_APS_MAX_ENDPOINTS_EXCEEDED,
  LRWPAN_STATUS_INDIRECT_BUFFER_FULL,
  LRPAN_STATUS_ZEP_FAILED,
  LRPAN_STATUS_ZEPCALLBACK_FAILED,
  LRPAN_STATUS_USRCALLBACK_FAILED,
  LRWPAN_STATUS_HEAPFULL
}LRWPAN_STATUS_ENUM;



typedef UINT16 SADDR;
//typedef UINT16 PANID;

//these bytes ALWAYS stored in little-endian order
typedef struct _LADDR {
  BYTE bytes[8];
}LADDR;

//only used to store IEEE Long Address or PAN short address
typedef union _LADDR_UNION {
  LADDR laddr;
  SADDR saddr;
}LADDR_UNION;


typedef enum _PHY_FREQ_ENUM {
  PHY_FREQ_868M=0,
  PHY_FREQ_RSV,
  PHY_FREQ_915M,
  PHY_FREQ_2405M  
}PHY_FREQ_ENUM;

typedef enum _NODE_TYPE_ENUM {
  NODE_TYPE_COORD=0,
  NODE_TYPE_ROUTER,
  NODE_TYPE_ENDDEVICE
}NODE_TYPE_ENUM;

//used for radio initialization
typedef union _RADIO_FLAGS {
	BYTE val;
	struct _RADIO_FLAGS_bits {
      unsigned listen_mode:1;       //if true, then put radio in listen mode, which
                                //is non-auto ack, no address decoding
       unsigned pan_coordinator:1;   //set the pan coordinator bit
	}bits;
 }RADIO_FLAGS;

typedef struct _MACPKT {
	BYTE *data;
	BYTE rssi;
}MACPKT;


#ifdef LRWPAN_COMPILER_BIG_ENDIAN
#define UINT32_LOWORD_LSB 3
#define UINT32_LOWORD_MSB 2
#define UINT32_HIWORD_LSB 1
#define UINT32_HIWORD_MSB 0
#define UINT16_LSB 1
#define UINT16_MSB 0
#else
#define UINT32_LOWORD_LSB 0
#define UINT32_LOWORD_MSB 1
#define UINT32_HIWORD_LSB 2
#define UINT32_HIWORD_MSB 3
#define UINT16_LSB 0
#define UINT16_MSB 1
#endif


#endif
