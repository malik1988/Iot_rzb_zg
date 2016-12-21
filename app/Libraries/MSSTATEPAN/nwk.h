/*
 * "Copyright (c) 2006 Robert B. Reese ("AUTHOR")"
 * All rights reserved.
 * (R. Reese, reese@ece.msstate.edu, Mississippi State University)
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
 *
 * This file makes use of Zigbee Alliance intellectual property in the
 * form of frame formats as specified in the Zigbee 1.0 standard.
 * The author consulted the
 * Zigbee standard when writing this file, and agreed to the terms
 * of conditions by the Zibee Alliance when downloading the standard.
 * As such, use of this file is governed by the Zigbee Trademarks,
 * Designations, and Logos policy. A paraphrase of this policy is
 * that free usage is granted for research, educational or personal
 * use. Commercial usage is restricted, see the policy for details.
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


#ifndef NWK_H
#define NWK_H

#define NWK_FRM_TYPE_DATA  0
#define NWK_FRM_TYPE_CMD   1
#define NWK_FRM_TYPE_MASK  3

#define NWK_IS_DATA(x) ((x & NWK_FRM_TYPE_MASK)==NWK_FRM_TYPE_DATA)
#define NWK_IS_CMD(x) ((x & NWK_FRM_TYPE_MASK)==NWK_FRM_TYPE_CMD)


#define NWK_SUPPRESS_ROUTE_DISCOVER  (0<<6)
#define NWK_ENABLE_ROUTE_DISCOVER    (1<<6)
#define NWK_FORCE_ROUTE_DISCOVER     (2<<6)

#define NWK_ROUTE_MASK         (3 << 6)
#define NWK_GET_ROUTE(x)       (x & NWK_ROUTE_MASK)


//this value of zero means that our packets will not
//be recognized as Zibee SPEC packets which is what we
//want.  Do not want these packets confused with packets
//from Zigbee compliant devices.
#define NWK_PROTOCOL           (0<<2)
#define NWK_PROTOCOL_MASK      (15 << 2)
#define NWK_GET_PROTOCOL(x)    ((x & NWK_PROTOCOL_MASK) >> 2)



#define NWK_SECURITY_MASK      (1 << 1)


#define NWK_GENERIC_RETRIES  3   //number of retries for NWK operations


#define NWK_RXBUFF_SIZE LRWPAN_MAX_NWK_RX_PKTS+1


typedef struct _NWK_FWD_PKT {
   BYTE *data;  //points to top of original pkt as it sits in the heap
   BYTE nwkOffset;  //start of the nwkheader in this packet
}NWK_FWD_PKT;


typedef struct _NWK_PIB{
	union _NWK_PIB_FLAGS{
		BYTE val;
		struct {
			unsigned nwkFormed:1;
		}bits;
	}flags;
#ifdef LRWPAN_FFD
	BYTE rxTail;             //tail pointer of rxBuff
    BYTE rxHead;             //head pointer of rxBuff
	//fifo for RX pkts, holds LRWPAN_MAX_NWK_RX_PKTS
	NWK_FWD_PKT  rxBuff[NWK_RXBUFF_SIZE];  //buffer for packets to be forwarded
#endif
}NWK_PIB;



typedef struct _NWK_RX_DATA {
	MACPKT orgpkt;
	BYTE nwkOffset;
	//parse these out of the packet for reference
	SADDR dstSADDR;  
   	SADDR srcSADDR; 
}NWK_RX_DATA;


typedef struct _NWK_TX_DATA {
	SADDR dstSADDR;
	BYTE  *dstLADDR;
   	SADDR srcSADDR;
	BYTE radius;
	BYTE fcflsb;
	BYTE fcfmsb;	
}NWK_TX_DATA;

typedef union _NWK_ARGS {
   struct {
		UINT32 ScanChannels;
		BYTE ScanDuration;
	}form_network;
   struct {
		UINT32 ScanChannels;
		BYTE ScanDuration;
		BOOL  RejoinNetwork;
   }join_network;
}NWK_ARGS;

typedef enum _NWK_STATE_ENUM {
  NWK_STATE_IDLE,
  NWK_STATE_COMMAND_START,
  NWK_STATE_GENERIC_TX_WAIT,
  NWK_STATE_FORM_NETWORK_START,
  NWK_STATE_JOIN_NETWORK_START,
  NWK_STATE_JOIN_NETWORK_START_WAIT,
  NWK_STATE_REJOIN_NETWORK_START,
  NWK_STATE_REJOIN_WAIT,
  NWK_STATE_JOIN_SEND_BEACON_REQ,
  NWK_STATE_JOIN_NWK_WAIT1_BREQ,
  NWK_STATE_JOIN_NWK_WAIT2_BREQ,
  NWK_STATE_JOIN_MAC_ASSOC_CHANSELECT,
  NWK_STATE_JOIN_MAC_ASSOC,
  NWK_STATE_JOIN_MAC_ASSOC_WAIT,
  NWK_STATE_FWD_WAIT
} NWK_STATE_ENUM;


typedef struct _NWK_SERVICE {
  LRWPAN_SVC_ENUM cmd;
  NWK_ARGS args;
  LRWPAN_STATUS_ENUM status;
}NWK_SERVICE;

extern NWK_SERVICE a_nwk_service;
extern NWK_TX_DATA a_nwk_tx_data;
extern NWK_STATE_ENUM nwkState;

extern NWK_RX_DATA a_nwk_rx_data;

UINT16 nwkGetHopsToDest(SADDR dstSADDR);

BOOL nwkRxBusy(void);
void nwkRxHandoff(void);

extern NWK_PIB nwk_pib;

void nwkFSM(void);
void nwkInit(void);

#define nwkIdle() (nwkState == NWK_STATE_IDLE)
#define nwkBusy() (nwkState != NWK_STATE_IDLE)

#define nwkDoService() \
   a_nwk_service.status = LRWPAN_STATUS_NWK_INPROGRESS;\
   nwkState = NWK_STATE_COMMAND_START;\
   nwkFSM();


#endif


