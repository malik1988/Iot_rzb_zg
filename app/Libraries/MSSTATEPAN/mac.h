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



#ifndef MAC_H
#define MAC_H

#define aBaseSlotDuration 60
#define aNumSuperFrameSlots 16
#define aBaseSuperFrameDuration (aBaseSlotDuration*aNumSuperFrameSlots)
#define aMaxBE 5
#define aMinBE 0
#define aUnitBackoffPeriod 20        //in symbols
#define macMaxCSMABackoffs 4
#define aMaxBeaconOverhead 75
#define aMaxBeaconPayloadLength (aMaxPHYPacketSize-aMaxBeaconOverhead)
#define aMaxFrameOverhead 25
#define aMaxFrameResponseTime 1220
#define aMaxFrameRetries LRWPAN_MAC_MAX_FRAME_RETRIES
#define aMaxLostBeacons 4
#define aMaxMACFrameSize (aMaxPHYPacketSize-aMaxFrameOverhead)
#define aResponseWaitTime (32*aBaseSuperFrameDuration)

//default timeout on network responses
#ifdef LRWPAN_DEBUG
//give longer due to debugging output
#define MAC_GENERIC_WAIT_TIME      MSECS_TO_MACTICKS(100)
#define MAC_ASSOC_WAIT_TIME        MAC_GENERIC_WAIT_TIME	//100ms
#define MAC_ORPHAN_WAIT_TIME       MAC_GENERIC_WAIT_TIME	//100ms
#else
#define MAC_GENERIC_WAIT_TIME      MSECS_TO_MACTICKS(20)	//20ms
#define MAC_ASSOC_WAIT_TIME        MAC_GENERIC_WAIT_TIME	//20ms
#define MAC_ORPHAN_WAIT_TIME       MAC_GENERIC_WAIT_TIME	//20ms
#endif



#define MAC_RXBUFF_SIZE LRWPAN_MAX_MAC_RX_PKTS+1

typedef struct _MAC_PIB {
	UINT32 macAckWaitDuration;
	union _MAC_PIB_flags {
		UINT32 val;
		struct {
			unsigned macAssociationPermit:1;
			unsigned macAutoRequest:1;
			unsigned macBattLifeExt:1;
			unsigned macGTSPermit:1;
			unsigned macPromiscousMode:1;
			unsigned macPanCoordinator:1;
			unsigned ackPending:1;
			unsigned TxInProgress:1;   //MAC TX FSM state
			unsigned GotBeaconResponse:1;      //set to a '1' when get Beacon Response
			unsigned WaitingForBeaconResponse:1; //set to '1' when waiting for Response
			unsigned macPending:1;       //mac CMD pending in the RX buffer
			unsigned macIsAssociated:1;
			unsigned WaitingForAssocResponse:1;
			unsigned GotOrphanResponse:1;
			unsigned WaitingForOrphanResponse:1;

		}bits;
	}flags;
	LADDR macCoordExtendedAddress;
	SADDR macCoordShortAddress;
	UINT16 macPANID;
	BYTE macDSN;
	BYTE depth;            //depth in the network
	BYTE macCapInfo;
    BYTE macMaxAckRetries;
	struct  {
		unsigned maxMaxCSMABackoffs:3;
		unsigned macMinBE:2;
	}misc;
	UINT32 tx_start_time;    //time that packet was sent
      UINT32 last_data_rx_time;    //time that last data rx packet was received that was accepted by this node

    BYTE bcnDepth;
	SADDR bcnSADDR;
	UINT16 bcnPANID;
	BYTE bcnRSSI;

	BYTE currentAckRetries;
	BYTE rxTail;             //tail pointer of rxBuff
    BYTE rxHead;             //head pointer of rxBuff
	//fifo for RX pkts, holds LRWPAN_MAX_MAC_RX_PKTS
	MACPKT  rxBuff[MAC_RXBUFF_SIZE];  //buffer for packets not yet processed

#ifdef LRWPAN_FFD
	//neighbor info
	UINT16 nextChildRFD;
	UINT16 nextChildRouter;
	BYTE   ChildRFDs;         //number of neighbor RFDs
	BYTE   ChildRouters;      //number of neighbor Routers
#endif


}MAC_PIB;


//used for parsing of RX data
typedef struct _MAC_RX_DATA {
	MACPKT *orgpkt;       //original packet
	BYTE fcflsb;
	BYTE fcfmsb;
	UINT16 DestPANID;
	LADDR_UNION DestAddr; //dst address, either short or long
	UINT16 SrcPANID;
	LADDR_UNION SrcAddr;  //src address, either short or long
	BYTE pload_offset;    //start of payload
}MAC_RX_DATA;

typedef struct _MAX_TX_DATA {
	UINT16 DestPANID;
	LADDR_UNION DestAddr; //dst address, either short or long
	UINT16 SrcPANID;
	SADDR SrcAddr;         //src address, either short or long, this holds short address version
	                       //if long is needed, then get this from HAL layer
	BYTE fcflsb;          //frame control bits specify header bits
	BYTE fcfmsb;
	union  {
		BYTE val;
		struct _MAC_TX_DATA_OPTIONS_bits {
			unsigned gts:1;
			unsigned indirect:1;
		}bits;
	}options;		
}MAC_TX_DATA;

typedef union _MAC_ARGS {
	struct {
		BYTE   LogicalChannel;
       //addressing, capinfo, security comes from mac_pib data.
	}assoc_req;
	struct {
		BYTE LogicalChannel;
	}beacon_req;
	struct {
       LRWPAN_STATUS_ENUM status;
	}error;
	struct {
		SADDR saddr;
	}ping_node;
}MAC_ARGS;

typedef enum _MAC_STATE_ENUM {
  MAC_STATE_IDLE,
  MAC_STATE_COMMAND_START,
  MAC_STATE_GENERIC_TX_WAIT,
  MAC_STATE_GENERIC_TX_WAIT_AND_UNLOCK,
  MAC_STATE_HANDLE_ORPHAN_NOTIFY,
  MAC_STATE_ORPHAN_WAIT1,
  MAC_STATE_ORPHAN_WAIT2,
  MAC_STATE_ASSOC_REQ_WAIT1,
  MAC_STATE_ASSOC_REQ_WAIT2,
  MAC_STATE_SEND_BEACON_RESPONSE,
  MAC_STATE_SEND_ASSOC_RESPONSE
 } MAC_STATE_ENUM;

typedef struct _MAC_SERVICE {
  LRWPAN_SVC_ENUM cmd;
  MAC_ARGS args;
  LRWPAN_STATUS_ENUM status;
}MAC_SERVICE;


extern MAC_PIB mac_pib;
extern MAC_SERVICE a_mac_service;
extern MAC_STATE_ENUM macState;
extern MAC_TX_DATA a_mac_tx_data;
extern MAC_RX_DATA a_mac_rx_data;


void macInit(void);
void macFSM(void);

LRWPAN_STATUS_ENUM macInitRadio(void);
LRWPAN_STATUS_ENUM macWarmStartRadio(void);
void macSetPANID(UINT16 panid);
UINT16 macGetPANID(void);
void macSetChannel(BYTE channel);
void macSetShortAddr(UINT16 saddr);
//SADDR macGetShortAddr();
#define macGetShortAddr()   (mac_addr_tbl[0].saddr)

BOOL macRxBuffEmpty(void);
BOOL macRxBuffFull(void);
MACPKT *macGetRxPacket(void);
void macFreeRxPacket(BOOL freemem);

#ifdef LRWPAN_FFD
BOOL usrJoinVerifyCallback(LADDR *ptr, BYTE capinfo);
BOOL usrJoinNotifyCallback(LADDR *ptr);
#endif


#define macIdle() (macState == MAC_STATE_IDLE)
#define macBusy() (macState != MAC_STATE_IDLE)

#define macTXIdle() (!mac_pib.flags.bits.TxInProgress)
#define macTXBusy() (mac_pib.flags.bits.TxInProgress)
#define macSetTxBusy() mac_pib.flags.bits.TxInProgress = 1
#define macSetTxIdle() mac_pib.flags.bits.TxInProgress = 0

#define macDoService() \
	a_mac_service.status = LRWPAN_STATUS_MAC_INPROGRESS;\
	macState = MAC_STATE_COMMAND_START;\
	macFSM();\


#endif

