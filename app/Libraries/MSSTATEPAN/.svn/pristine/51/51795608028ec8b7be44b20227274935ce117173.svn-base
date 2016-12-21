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
V0.21 Fixed problem in nwkCopyFwdPkt relating to copying of the radius byte
       27/July/2006  RBR
V0.2 added PC-based binding         21/July/2006  RBR
V0.1 Initial Release                10/July/2006  RBR

*/


/*
Network Layer


*/

#include "compiler.h"
#include "lrwpan_config.h"         //user configurations
#include "lrwpan_common_types.h"   //types common acrosss most files
#include "ieee_lrwpan_defs.h"
#include "console.h"
#include "debug.h"
#include "memalloc.h"
#include "neighbor.h"
#include "hal.h"
#include "halStack.h"
#include "phy.h"
#include "mac.h"
#include "nwk.h"
#include "aps.h"

#include "neighbor.h"



typedef enum _NWK_RXSTATE_ENUM {
	NWK_RXSTATE_IDLE,
	NWK_RXSTATE_START,
	NWK_RXSTATE_APS_HANDOFF,
	NWK_RXSTATE_DOROUTE
} NWK_RXSTATE_ENUM;

static NWK_RXSTATE_ENUM nwkRxState;
static void nwkParseHdr(BYTE *ptr);


NWK_SERVICE a_nwk_service;
NWK_STATE_ENUM nwkState;
NWK_RX_DATA a_nwk_rx_data;

BYTE nwkDSN;

NWK_PIB nwk_pib;

static void nwkRxFSM(void);


//locals
#ifndef LRWPAN_COORDINATOR
static UINT32 nwk_utility_timer;   //utility timer
static UINT8 nwk_retries;       //utility retry counter
#endif

#ifdef LRWPAN_FFD
void nwkCopyFwdPkt(void);
static BOOL nwkRxBuffFull(void);
static BOOL nwkRxBuffEmpty(void);
static NWK_FWD_PKT *nwkGetRxPacket(void);
static void nwkFreeRxPacket(BOOL freemem);
#endif



//there can only be one TX in progress at a time, so
//a_aps_tx_data contains the arguments for that TX on the NWK layer.
NWK_TX_DATA a_nwk_tx_data;

void nwkTxData(BOOL fwdFlag);

void nwkInit(void){
	nwkDSN = 0;
	nwk_pib.flags.val = 0;
	nwkState = NWK_STATE_IDLE;
	nwkRxState= NWK_RXSTATE_IDLE;
#ifdef LRWPAN_FFD
	nwk_pib.rxTail = 0;
	nwk_pib.rxHead = 0;
#endif
}

void nwkFSM(void){



	macFSM();
	nwkRxFSM();

nwkFSM_start:

	switch (nwkState) {
	 case NWK_STATE_IDLE:
#ifdef LRWPAN_FFD
		 //see if we have packets to forward and can grab the TX buffer
		 if (!nwkRxBuffEmpty() && phyTxUnLocked()) {

			 //grab the lock and forward the packet
			 phyGrabTxLock();
			 nwkCopyFwdPkt();
			 //transmit it
			 nwkTxData(TRUE); //use TRUE as this is a forwarded packet
			 nwkState = NWK_STATE_FWD_WAIT;

		 }

#endif
		 break;

	 case NWK_STATE_COMMAND_START:
		 switch(a_nwk_service.cmd) {
	 case LRWPAN_SVC_NWK_GENERIC_TX:
		 //at this point, need to check  if INDIRECT or DIRECT, take action
		 //send a generic packet with arguments specified by upper level
		 //this assumes the upper level has grabbed the TX buffer lock
		 nwkTxData(FALSE);
		 nwkState = NWK_STATE_GENERIC_TX_WAIT;
		 break;
#ifdef LRWPAN_COORDINATOR
	 case LRWPAN_SVC_NWK_FORM_NETWORK:
		 //if forming network, restart everything
		 //if we are forming a network, then we need to restart
		 //the PHY, MAC layers from scratch
		 phyInit();
		 macInit();

		 ntInitTable();  //init neighbor table

		 a_nwk_service.status = macInitRadio();  //turns on the radio
		 if (a_nwk_service.status != LRWPAN_STATUS_SUCCESS) {
			 DEBUG_STRING(DBG_ERR, "NWK Formation failed!\n");
			 nwkState = NWK_STATE_IDLE;
			 break;
		 }

		 //this is where we form a network.
		 nwkState = NWK_STATE_FORM_NETWORK_START;

		 goto nwkFSM_start;
#else

	 case LRWPAN_SVC_NWK_JOIN_NETWORK:
		 // see if this is a rejoin or join
		 if (a_nwk_service.args.join_network.RejoinNetwork) {
			 mac_pib.flags.bits.macIsAssociated = 0; //if doing rejoin, unsure of association, clear it.
			 nwkState = NWK_STATE_REJOIN_NETWORK_START;
			 goto nwkFSM_start;  //do not do any other initialization
		 }
		 else nwkState = NWK_STATE_JOIN_NETWORK_START;
		 //if joining/rejoining network, restart everything
		 //if we are forming a network, then we need to restart
		 //the PHY, MAC layers from scratch
		 phyInit();
		 macInit();
#ifdef LRWPAN_FFD
		 ntInitTable();  //init neighbor table
#endif

		 a_nwk_service.status = macInitRadio();  //turns on the radio
		 if (a_nwk_service.status != LRWPAN_STATUS_SUCCESS) {
			 DEBUG_STRING(DBG_ERR, "NWK JOIN/REJOIN failed!\n");
			 nwkState = NWK_STATE_IDLE;
			 break;
		 }

		 goto nwkFSM_start;
#endif



	 default: break;

		 }//end switch(a_nwk_service.cmd)

	 case NWK_STATE_GENERIC_TX_WAIT:
		 if (macBusy()) break;
		 //mac finished, copy status.
		 a_nwk_service.status = a_mac_service.status;
		 nwkState = NWK_STATE_IDLE;
		 break;


#ifdef LRWPAN_FFD
	 case NWK_STATE_FWD_WAIT:
		 if (macBusy()) break;
		 //mac finished, this is only used for fwding packets
		 //will ignore status for now since there is not much
		 //can do about it. Eventally, with more advanced routing,
		 // will record the status of this link and try an alternate
		 //route on failure.
		 phyReleaseTxLock(); //release the lock
		 nwkState = NWK_STATE_IDLE;
		 break;
#endif

	 case NWK_STATE_REJOIN_NETWORK_START:
		 if (macBusy()) break;
		 //send an orphan notification
		 a_mac_service.cmd = LRWPAN_SVC_MAC_ORPHAN_NOTIFY;          
		 nwkState = NWK_STATE_REJOIN_WAIT;      
		 macDoService();        
		 break;

	 case NWK_STATE_REJOIN_WAIT:
		 if (macBusy()) break;
		 //at this point, rejoin is finished, get status
		 a_nwk_service.status = a_mac_service.status;
		 nwkState = NWK_STATE_IDLE;
		 break;

#ifdef LRWPAN_COORDINATOR
	 case NWK_STATE_FORM_NETWORK_START:

		 //here is where we would scan the channels, etc.
		 //instead, we will just set the channel, and indicate
		 //that the network is formed, and init the neighbor table.
		 //PANID used is whatever MACPAN ID is set to
		 macSetChannel(LRWPAN_DEFAULT_START_CHANNEL);
		 macSetShortAddr(0);
		 //initialize address assignment, must be done after the
		 //short address is set
		 ntInitAddressAssignment();


		 //add ourselves to the 
		 nwk_pib.flags.bits.nwkFormed = 1;
		 mac_pib.flags.bits.macIsAssociated = 1; //I am associated with myself!
		 //tell MAC to allow association
		 mac_pib.flags.bits.macAssociationPermit = 1;
		 a_nwk_service.status = LRWPAN_STATUS_SUCCESS;

		 nwkState = NWK_STATE_IDLE;
		 break;

#endif
#ifndef LRWPAN_COORDINATOR
	 case NWK_STATE_JOIN_NETWORK_START:
		 //if trying to join, do not allow association
		 mac_pib.flags.bits.macAssociationPermit = 0;

#ifdef LRWPAN_FORCE_ASSOCIATION_TARGET
		 //if forcing association to particular target, skip beacon request
		 //go to state that will start forced association, selecting channels
		 nwkState = NWK_STATE_JOIN_MAC_ASSOC_CHANSELECT;
#else
		 //select a channel
		 a_mac_service.args.beacon_req.LogicalChannel = LRWPAN_DEFAULT_START_CHANNEL;
		 //set retries
		 //we always send out at least three BEACON req to make
		 //sure that we adequately poll everybody in the region
		 nwk_retries = NWK_GENERIC_RETRIES;
		 mac_pib.bcnDepth = 0xFF;  //initialze the beacon depth response
		 //start the request
		 nwkState = NWK_STATE_JOIN_SEND_BEACON_REQ;
#endif
		 goto nwkFSM_start;

	 case NWK_STATE_JOIN_SEND_BEACON_REQ:
		 //at this point, we would start scanning channels
		 //sending out beacon requests on each channel
		 // we will only send out a beacon on the default channel, instead of scanning
		 if (macBusy()) break;
		 a_mac_service.cmd = LRWPAN_SVC_MAC_BEACON_REQ;          
		 nwkState = NWK_STATE_JOIN_NWK_WAIT1_BREQ;     
		 macDoService();     
		 break;

		 //waits for BCN request TX to finish
	 case NWK_STATE_JOIN_NWK_WAIT1_BREQ:
		 if (macBusy()) break;
		 //at this point, the packet has been sent. Now have
		 //to wait for a response. Record
		 nwk_utility_timer = halGetMACTimer();
		 nwkState =NWK_STATE_JOIN_NWK_WAIT2_BREQ;
		 break;

	 case NWK_STATE_JOIN_NWK_WAIT2_BREQ:
		 //wait for either a BCN response or timeout.
		 if (mac_pib.flags.bits.GotBeaconResponse) {
			 //we have received a BCN response. Try joining this node

			 //for right now just print out debug message
			 DEBUG_STRING(DBG_INFO,"Got BCN Response\n");
			 //keep trying, want to poll everybody in range
			 //and pick the closest one
			 mac_pib.flags.bits.GotBeaconResponse = 0;
		 }else if (halMACTimerNowDelta(nwk_utility_timer)> MSECS_TO_MACTICKS(LRWPAN_NWK_JOIN_WAIT_DURATION) ) {
			 if (nwk_retries) nwk_retries--;
			 if (nwk_retries) {
				 //retry Beacon Request
				 nwkState = NWK_STATE_JOIN_SEND_BEACON_REQ;
				 goto nwkFSM_start;
			 }else 
			 {
				 //out of retries, check bcnDepth
				 if ( mac_pib.bcnDepth != 0xFF) {
					 //we got a response, so lets try to join
					 nwkState = NWK_STATE_JOIN_MAC_ASSOC;
					 //use the same channel
					 a_mac_service.args.assoc_req.LogicalChannel  = a_mac_service.args.beacon_req.LogicalChannel;
                               macSetPANID(mac_pib.bcnPANID);  //use beacon response as PANID
					 DEBUG_STRING(DBG_INFO,"NWK trying association\n");
				 }else {
					 //indicate failure
					 DEBUG_STRING(DBG_INFO,"NWK Join Timeout\n");
					 a_nwk_service.status = LRWPAN_STATUS_NWK_JOIN_TIMEOUT;
					 nwkState = NWK_STATE_IDLE;
				 }

				 //clear flags
				 mac_pib.flags.bits.GotBeaconResponse = 0;
				 mac_pib.flags.bits.WaitingForBeaconResponse = 0;                
			 }
		 }
		 break;

	 case NWK_STATE_JOIN_MAC_ASSOC_CHANSELECT:
		 //this will eventually scan channels, for now, just select default
		 a_mac_service.args.assoc_req.LogicalChannel = LRWPAN_DEFAULT_START_CHANNEL;
		 nwkState = NWK_STATE_JOIN_MAC_ASSOC;
		 goto nwkFSM_start;

	 case NWK_STATE_JOIN_MAC_ASSOC:
		 //do association to PANID discovered by beacon request
		 if (macBusy()) break;
		 a_mac_service.cmd = LRWPAN_SVC_MAC_ASSOC_REQ;
		 nwkState = NWK_STATE_JOIN_MAC_ASSOC_WAIT;
		 macDoService();

		 break;

	 case NWK_STATE_JOIN_MAC_ASSOC_WAIT:
		 if (macBusy()) break;
		 //at this point, association is finished, get status
		 a_nwk_service.status = a_mac_service.status;
#ifdef LRWPAN_FFD
		 if (a_nwk_service.status == LRWPAN_STATUS_SUCCESS) {
			 //as a router, initialize address assignment and
			 //begin allowing association
			 ntInitAddressAssignment();
			 mac_pib.flags.bits.macAssociationPermit = 1;
		 }
#endif
		 nwkState = NWK_STATE_IDLE;
		 break;

#endif


		 //these states for FORM NETWORK
	 default:  break;


	}//end switch(nwkState)


}

//Add the NWK header, then send it to MAC
//if fwdFlag is true, then packet is being forwarded, so nwk header
//is already in place, and assume that currentTxFrm and currentTxPLen
//are correct as well, and that the radius byte has been decremented.
void nwkTxData(BOOL fwdFlag) {



	//if we are not associated, don't bother sending NWK packet
	if (!mac_pib.flags.bits.macIsAssociated) {
		//call a dummy service that just returns an error code
		//have to do it this way since the caller is expecting a
		//mac service call
		a_mac_service.args.error.status = LRWPAN_STATUS_MAC_NOT_ASSOCIATED;
		a_mac_service.cmd = LRWPAN_SVC_MAC_ERROR;
		goto nwkTxData_sendit;
	}

	if (a_nwk_tx_data.radius == 0) {
		DEBUG_STRING(DBG_ERR,"Nwk Radius is zero, discarding packet.\n");
		//can no longer forward this packet.
		a_mac_service.args.error.status =  LRWPAN_STATUS_NWK_RADIUS_EXCEEDED;
		a_mac_service.cmd = LRWPAN_SVC_MAC_ERROR;
		goto nwkTxData_sendit;    
	}


	if (fwdFlag) goto nwkTxData_addmac;
	//sequence number
	--phy_pib.currentTxFrm;
	*phy_pib.currentTxFrm = nwkDSN;
	nwkDSN++;

	//radius, decrement before sending, receiver will
	//get a value that is one less than this node.
	--phy_pib.currentTxFrm;
	*phy_pib.currentTxFrm = (--a_nwk_tx_data.radius);

	//src address
	--phy_pib.currentTxFrm;
	*phy_pib.currentTxFrm = (BYTE) (a_nwk_tx_data.srcSADDR >> 8);
	--phy_pib.currentTxFrm;
	*phy_pib.currentTxFrm = (BYTE) (a_nwk_tx_data.srcSADDR);

	//dst address
	--phy_pib.currentTxFrm;
	*phy_pib.currentTxFrm = (BYTE) (a_nwk_tx_data.dstSADDR >> 8);
	--phy_pib.currentTxFrm;
	*phy_pib.currentTxFrm = (BYTE) (a_nwk_tx_data.dstSADDR);

	//frame control
	--phy_pib.currentTxFrm;
	*phy_pib.currentTxFrm = a_nwk_tx_data.fcfmsb;
	--phy_pib.currentTxFrm;
	*phy_pib.currentTxFrm = a_nwk_tx_data.fcflsb;

	//network header is fixed size
	phy_pib.currentTxFlen +=  8;

nwkTxData_addmac:
	//fill in the MAC fields. For now, we don't support inter-PAN
	// so the PANID has to be our mac PANID
	a_mac_tx_data.DestPANID = mac_pib.macPANID;
	a_mac_tx_data.SrcPANID = mac_pib.macPANID;

	if (a_nwk_tx_data.dstSADDR == LRWPAN_SADDR_USE_LADDR ){
		//long address is specified from above.  We assume they know where
		//they are going no routing necessary
		a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_LADDR|LRWPAN_FCF_SRCMODE_LADDR;
		//copy in the long address
		halUtilMemCopy(&a_mac_tx_data.DestAddr.laddr.bytes[0], a_nwk_tx_data.dstLADDR, 8);
	} else {
		//lets do some routing
#ifdef LRWPAN_RFD
		//RFD's are easy. Always send to parent, our SRC address is always long
		//so that parent can confirm that the RFD is still in their neighbor table
		//will use the parent short address
		a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_SADDR|LRWPAN_FCF_SRCMODE_LADDR;
		a_mac_tx_data.DestAddr.saddr = mac_pib.macCoordShortAddress;
#else
		{
			SADDR newDstSADDR;
			//this is router. need to determine the new dstSADDR
			newDstSADDR = a_nwk_tx_data.dstSADDR; //default
			DEBUG_STRING(DBG_INFO,"Routing pkt to: ");
            DEBUG_UINT16(DBG_INFO,newDstSADDR);
			if (a_nwk_tx_data.dstSADDR != LRWPAN_BCAST_SADDR) {
				//not broadcast address
				newDstSADDR = ntFindNewDst(a_nwk_tx_data.dstSADDR);
				DEBUG_STRING(DBG_INFO," through: ");
                DEBUG_UINT16(DBG_INFO,newDstSADDR);
				if (newDstSADDR == LRWPAN_BCAST_SADDR) {
					DEBUG_STRING(DBG_INFO,", UNROUTABLE, error!\n ");
					//error indicator. An unroutable packet from here.
					a_mac_service.args.error.status = LRWPAN_STATUS_NWK_PACKET_UNROUTABLE;
					a_mac_service.cmd = LRWPAN_SVC_MAC_ERROR;
					goto nwkTxData_sendit;
				}
				DEBUG_STRING(DBG_INFO,"\n");
			}

			//fill it in.
			a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_SADDR|LRWPAN_FCF_SRCMODE_LADDR;
			a_mac_tx_data.DestAddr.saddr = newDstSADDR;
		}
#endif

	}


	//for data frames, we want a MAC level ACK, unless it is a broadcast.
	if ( ((LRWPAN_GET_DST_ADDR(a_mac_tx_data.fcfmsb)) == LRWPAN_ADDRMODE_SADDR) &&
		a_mac_tx_data.DestAddr.saddr == LRWPAN_BCAST_SADDR) {
			//no MAC ACK
			a_mac_tx_data.fcflsb = LRWPAN_FRAME_TYPE_DATA|LRWPAN_FCF_INTRAPAN_MASK ;
		}else {
			a_mac_tx_data.fcflsb = LRWPAN_FRAME_TYPE_DATA|LRWPAN_FCF_INTRAPAN_MASK |LRWPAN_FCF_ACKREQ_MASK;
		}

		//send it.
		a_mac_service.cmd = LRWPAN_SVC_MAC_GENERIC_TX;



nwkTxData_sendit:

		macDoService();

}



static void nwkRxFSM(void) {
	BYTE *ptr;

nwkRxFSM_start:

	switch(nwkRxState) {
		case NWK_RXSTATE_IDLE:
			break;
		case NWK_RXSTATE_START:
			//we have a packet, lets check it out.
			ptr = a_nwk_rx_data.orgpkt.data + a_nwk_rx_data.nwkOffset;
			if (NWK_IS_CMD(*ptr)) {
				//currently don't handle CMD packets. Discard.
				DEBUG_STRING(DBG_INFO,"NWK: Received NWK CMD packet, discarding.\n");
				//MAC resource already free; need to free the MEM resource
				MemFree(a_nwk_rx_data.orgpkt.data);
				nwkRxState = NWK_RXSTATE_IDLE;
				break;
			}
			//this is a data packet. do more parsing.
			nwkParseHdr(ptr);

			//see if this is for us.
			if ((a_nwk_rx_data.dstSADDR == LRWPAN_BCAST_SADDR) ||
				(a_nwk_rx_data.dstSADDR == LRWPAN_SADDR_USE_LADDR) ||
				(a_nwk_rx_data.dstSADDR == macGetShortAddr())) {
					//hand this off to the APS layer
					nwkRxState = NWK_RXSTATE_APS_HANDOFF;
				} else {
					//have to route this packet
					nwkRxState = NWK_RXSTATE_DOROUTE;
				}
				goto nwkRxFSM_start;

		case NWK_RXSTATE_APS_HANDOFF:
			if (apsRxBusy()) break;    //apsRX is still busy
			//handoff the current packet
			apsRxHandoff();
			//we are finished with this packet.
			//we don't need to do anything to free this resource other
			// than to change state
			nwkRxState = NWK_RXSTATE_IDLE;
			break;


		case NWK_RXSTATE_DOROUTE:
#ifdef LRWPAN_RFD
			//RFD somehow got a data packet not intended for it.
			//should never happen, but put code here anyway to discard it.
			DEBUG_STRING(DBG_INFO,"NWK: RFD received spurious datapacket, discarding.\n");
			MemFree(a_nwk_rx_data.orgpkt.data);
			nwkRxState = NWK_RXSTATE_IDLE;
#else
			//first, check the radius, if zero, then discard.
			if (!(*(ptr+6))) {
				DEBUG_STRING(DBG_INFO,"NWK: Data packet is out of hops for dest: ");
				DEBUG_UINT16(DBG_INFO,a_nwk_rx_data.dstSADDR);
                DEBUG_STRING(DBG_INFO,", discarding...\n");
				MemFree(a_nwk_rx_data.orgpkt.data);
				nwkRxState = NWK_RXSTATE_IDLE;
				break;
			}
			DEBUG_STRING(DBG_INFO,"NWK: Routing NWK Packet to: ");
			DEBUG_UINT16(DBG_INFO,a_nwk_rx_data.dstSADDR);
			DEBUG_STRING(DBG_INFO,"\n");
			//this packet requires routing, not destined for us.
			if (nwkRxBuffFull()) {
				//no more room. discard this packet
				DEBUG_STRING(DBG_INFO,"NWK: FWD buffer full, discarding pkt.\n");
				DEBUG_STRING(DBG_INFO,"NWK state: ");
				DEBUG_UINT8(DBG_INFO,nwkState);
				DEBUG_STRING(DBG_INFO,"MAC state: ");
				DEBUG_UINT8(DBG_INFO,macState);
				DEBUG_STRING(DBG_INFO,"\n");
				MemFree(a_nwk_rx_data.orgpkt.data);
				nwkRxState = NWK_RXSTATE_IDLE;
			}else {
				//ok, add this pkt to the buffer
				nwk_pib.rxHead++;
				if (nwk_pib.rxHead == NWK_RXBUFF_SIZE) nwk_pib.rxHead = 0;
				//save it.
				nwk_pib.rxBuff[nwk_pib.rxHead].data = a_nwk_rx_data.orgpkt.data;
				nwk_pib.rxBuff[nwk_pib.rxHead].nwkOffset = a_nwk_rx_data.nwkOffset;
				nwkRxState = NWK_RXSTATE_IDLE;
				//this packet will be retransmitted by nwkFSM
			}

#endif

			break;

		default:
			break;

	}


}



//Callback from MAC Layer
//Returns TRUE if nwk is still busy with last RX packet.

BOOL nwkRxBusy(void){
	return(nwkRxState != NWK_RXSTATE_IDLE);
}

//Callback from MAC Layer
//Hands off parsed packet from MAC layer, frees MAC for parsing
//next packet.
void nwkRxHandoff(void){

	a_nwk_rx_data.orgpkt.data = a_mac_rx_data.orgpkt->data;
	a_nwk_rx_data.orgpkt.rssi = a_mac_rx_data.orgpkt->rssi;
	a_nwk_rx_data.nwkOffset = a_mac_rx_data.pload_offset;
	nwkRxState = NWK_RXSTATE_START;
}

static void nwkParseHdr(BYTE *ptr) {

	//ptr is pointing at nwk header. Get the SRC/DST nodes.
	ptr= ptr+2;
	//get Dst SADDR
	a_nwk_rx_data.dstSADDR = *ptr;
	ptr++;
	a_nwk_rx_data.dstSADDR += (((UINT16)*ptr) << 8);
	ptr++;

	//get Src SADDR
	a_nwk_rx_data.srcSADDR = *ptr;
	ptr++;
	a_nwk_rx_data.srcSADDR += (((UINT16)*ptr) << 8);
	ptr++;


}

#ifdef LRWPAN_FFD

//copies packet to forward from heap space to TXbuffer space
void nwkCopyFwdPkt(void){
	BYTE *srcptr, len;
	NWK_FWD_PKT *pkt;

	phy_pib.currentTxFrm = &tmpTxBuff[LRWPAN_MAX_FRAME_SIZE];
	//get next PKT
	pkt = nwkGetRxPacket();
	
	srcptr = pkt->data;  //points at original packet in heapspace

	//compute bytes to copy.
	//nwkoffset is the offset of the nwkheader in the original packet
	len = *(srcptr) - pkt->nwkOffset - PACKET_FOOTER_SIZE + 1 ;


	//point this one byte past the end of the packet
	srcptr = srcptr
		+ *(srcptr) //length of original packet, not including this byte
		+ 1         //add one for first byte which contains packet length
		- PACKET_FOOTER_SIZE; //subtract footer bytes, don't want to copy these.
	//save length
	phy_pib.currentTxFlen = len;
	//copy from heap space to TXBuffer space
	do {
		srcptr--; phy_pib.currentTxFrm--;
		*phy_pib.currentTxFrm = *srcptr;
		len--;
	}while(len);
	nwkFreeRxPacket(TRUE);  //free this packet
	//some final steps
	//get the dstSADDR, needed for routing.
	a_nwk_tx_data.dstSADDR = *(phy_pib.currentTxFrm+2);
	a_nwk_tx_data.dstSADDR += (((UINT16)*(phy_pib.currentTxFrm+3)) << 8);

	//decrement the radius before sending it on.
	*(phy_pib.currentTxFrm+6)= *(phy_pib.currentTxFrm+6)- 1;
	a_nwk_tx_data.radius = *(phy_pib.currentTxFrm+6);

	
	//leave the SADDR unchanged as we want to know where this originated from!
#if 0
	//replace the SADDR with our SADDR
	*(phy_pib.currentTxFrm+4) = (BYTE) macGetShortAddr();
	*(phy_pib.currentTxFrm+5) = (BYTE) (macGetShortAddr() >>8);
#endif 

}

static BOOL nwkRxBuffFull(void){
	BYTE tmp;
	//if next write would go to where Tail is, then buffer is full
	tmp = nwk_pib.rxHead+1;
	if (tmp == NWK_RXBUFF_SIZE) tmp = 0;
	return(tmp == nwk_pib.rxTail);
}

static BOOL nwkRxBuffEmpty(void){
	return(nwk_pib.rxTail == nwk_pib.rxHead);
}

//this does NOT remove the packet from the buffer
static NWK_FWD_PKT *nwkGetRxPacket(void) {
	BYTE tmp; 
	if (nwk_pib.rxTail == nwk_pib.rxHead) return(NULL);
	tmp = nwk_pib.rxTail+1;
	if (tmp == NWK_RXBUFF_SIZE) tmp = 0;
	return(&nwk_pib.rxBuff[tmp]);
}

//frees the first packet in the buffer.
static void nwkFreeRxPacket(BOOL freemem) {
	nwk_pib.rxTail++;
	if (nwk_pib.rxTail == NWK_RXBUFF_SIZE) nwk_pib.rxTail = 0;
	if (freemem) MemFree(nwk_pib.rxBuff[nwk_pib.rxTail].data);
}

#endif

//given a router child SADDR, find the parent router SADDR
UINT16 nwkFindParentSADDR(SADDR childSADDR) {

	UINT8 currentDepth;
	SADDR currentParent;
	SADDR currentRouter;
	SADDR maxSADDR;
	UINT8 i;


	currentDepth = 1;
	currentParent = 0;
	do {
		for (i=0; i<LRWPAN_MAX_ROUTERS_PER_PARENT; i++) {
			if (i==0) currentRouter = currentParent+1;
			else currentRouter += ntGetCskip(currentDepth);
			if (childSADDR == currentRouter) return(currentRouter);
			maxSADDR = ntGetMaxSADDR(currentRouter,currentDepth+1);
			if ((childSADDR > currentRouter) && (childSADDR <= maxSADDR)) 
				break; //must go further down the tree
		}
		currentDepth++;
		currentParent = currentRouter;
	}
	while (currentDepth < LRWPAN_MAX_DEPTH-1);  
	//if we reach here, could not find an address. Return 0 as an error
	return(0);
}

UINT16 nwkGetHopsToDest(SADDR dstSADDR){

	UINT16 numHops;
	SADDR currentParent, maxSADDR;
	UINT8 currentDepth;
	UINT8 i;
	SADDR currentRouter;

	numHops = 1;            //return a minimum value of 1

	currentDepth = 0;
	//first compute hops up the tree then down the tree
	if ( macGetShortAddr() == 0) goto nwkGetHopsToDest_down;  //this is the coordinator
	if (macGetShortAddr() == dstSADDR) return(1);  //to myself, should not happen, but return min value
	currentParent = mac_pib.macCoordShortAddress; //start with my parent address
	currentDepth = mac_pib.depth - 1; //depth of my parent.
	do {
		if (currentParent == dstSADDR) return(numHops);  //destination is one of my parent nodes.
		if (currentParent == 0) break;         //at coordinator.
		//compute the max SADDR address range of parent

		maxSADDR = ntGetMaxSADDR(currentParent,currentDepth+1);  //depth of parent's children
		if ((dstSADDR > currentParent) &&  (dstSADDR <= maxSADDR)) {
			//this address is in this router's range, stop going up.
			break;
		}
		//go up a level
		currentDepth--;
		numHops++;
		if (currentDepth == 0 ) currentParent =0;
		else { currentParent = nwkFindParentSADDR(currentParent);
		if (!currentParent) {
			//could not find, set numHops to maximum and return
			return(LRWPAN_MAX_DEPTH<<1);
		}
		}
	}while(1);

nwkGetHopsToDest_down:
	currentDepth++; //increment depth, as this should reflect my current children
	//now search going down.
	do {
		//destination is in the current parent's range
		//see if it is one of the routers or children.
		//first see if it is one of the children of current parent
		numHops++;
		maxSADDR = ntGetMaxSADDR(currentParent,currentDepth);
		if (dstSADDR > (maxSADDR-LRWPAN_MAX_NON_ROUTER_CHILDREN) &&
			dstSADDR <= maxSADDR) break;  //it is one of the children nodes
		for (i=0; i<LRWPAN_MAX_ROUTERS_PER_PARENT; i++) {
			if (i==0) currentRouter = currentParent+1;
			else currentRouter += ntGetCskip(currentDepth);

			if (dstSADDR == currentRouter) return(currentRouter);
			maxSADDR = ntGetMaxSADDR(currentRouter,currentDepth+1);
			if ((dstSADDR > currentRouter) && (dstSADDR <= maxSADDR)) 
				break; //must go further down the tree
		}
		if (i == LRWPAN_MAX_ROUTERS_PER_PARENT) {
			//must be one of my non-router children, increment hops, return
			return(numHops);
		}
		currentDepth++;
		currentParent = currentRouter; 

	}while(currentDepth < LRWPAN_MAX_DEPTH-1); 

	if (numHops > LRWPAN_NWK_MAX_RADIUS) {
		DEBUG_STRING(DBG_ERR,"nwkGetHopsToDest: Error in hop calculation: ");
		DEBUG_UINT8(DBG_ERR,numHops);
		DEBUG_STRING(DBG_ERR,"\n");
		numHops = LRWPAN_NWK_MAX_RADIUS-1;
	}
	return(numHops);
}







