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



/*
APS Layer


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
#include "zep.h"
#include "aps.h"
#include "evboard.h"



#define apsTXIdle() (!aps_pib.flags.bits.TxInProgress)
#define apsTXBusy() (aps_pib.flags.bits.TxInProgress)
#define apsSetTxBusy() aps_pib.flags.bits.TxInProgress = 1
#define apsSetTxIdle() aps_pib.flags.bits.TxInProgress = 0


typedef enum _APS_RXSTATE_ENUM {
    APS_RXSTATE_IDLE,
    APS_RXSTATE_START,
    APS_RXSTATE_RESOLVE_INDIRECT,
    APS_RXSTATE_ACK_SEND_WAIT
} APS_RXSTATE_ENUM;



APS_PIB aps_pib;
APS_SERVICE a_aps_service;
APS_STATE_ENUM apsState;
APS_EP_ELEM apsEndPoints[LRWPAN_MAX_ENDPOINTS];

//there can only be one TX in progress at a time, so
//a_aps_tx_data contains the arguments for that TX on the APS layer.
APS_TX_DATA a_aps_tx_data;
APS_RX_DATA a_aps_rx_data;
APS_RX_DATA indirect_data;  //used for holding an indirect packet while being reflected.

void apsFSM(void);
void apsTxData(BOOL copy_payload);

//locals
static APS_RXSTATE_ENUM apsRxState;
static LRWPAN_STATUS_ENUM apsTxFSM_status;

static void apsParseHdr(BYTE *ptr);
static void apsRxFSM(void);
static BOOL apsCheckAck(void);
static void apsFormatAck(void);
static void apsTxFSM(void);
static void apsInjectPacket(BOOL indirect_flag);
static void apsInjectTxPacket(void);

#ifdef LRWPAN_COORDINATOR
void apsRxBuffInit(void);
BOOL apsRxBuffFull(void);
BOOL apsRxBuffEmpty(void);
APS_RX_DATA *apsGetRxPacket(void);
void apsFreeRxPacket(BOOL freemem);
void apsRxBuffAdd (APS_RX_DATA *ptr);

#endif

//converts MAC TICKs to Microseconds
UINT32 aplMacTicksToUs(UINT32 ticks)
{

    return(halMacTicksToUs(ticks));

}

//this sleeps, then restarts the stack and the radio
void aplShutdown(void)
{
    //wait until stack is idle
    while(apsBusy()) apsFSM();
    //DISABLE_GLOBAL_INTERRUPT();  //disable interrupts, if halSleep wants to reenable them, let it.
    halShutdown();
    a_aps_service.status = LRWPAN_STATUS_SUCCESS;

}

void aplWarmstart(void)
{
    a_aps_service.status = macWarmStartRadio();
}



//this is top level init, does inits for other layers
void apsInit(void)
{

    debug_level = 0;
    apsState = APS_STATE_IDLE;
    apsRxState = APS_RXSTATE_IDLE;
#ifdef LRWPAN_COORDINATOR
    aps_pib.rxCnt = 0;
    aps_pib.rxTail = 0;
    aps_pib.rxHead = 0;
#endif

    aps_pib.apsTSN = 0;
    aps_pib.flags.val = 0;
    aps_pib.apscAckWaitDuration = MSECS_TO_MACTICKS(LRWPAN_APS_ACK_WAIT_DURATION) ; //convert to MAC Ticks,200ms
    aps_pib.apsAckWaitMultiplier = 1; //default value
    aps_pib.apscMaxFrameRetries = LRWPAN_APS_MAX_FRAME_RETRIES ;
    //initialize the endpoints
    aps_pib.activeEPs = 0;  //only tracks user endpoints.
    phyInit();
    macInit();
    nwkInit();
    zepInit();

}

//simply remembers these so that we can report them to the coordinator.
LRWPAN_STATUS_ENUM aplRegisterEndPoint(BYTE epNum)
{
    if (aps_pib.activeEPs == LRWPAN_MAX_ENDPOINTS) return(LRWPAN_STATUS_APS_MAX_ENDPOINTS_EXCEEDED);
    if (epNum == 0) return(LRWPAN_STATUS_APS_ILLEGAL_ENDPOINT);
    apsEndPoints[aps_pib.activeEPs++].epNum = epNum;
    return(LRWPAN_STATUS_SUCCESS);
}

//Formats a Send Message service
void aplFmtSendMSG (BYTE dstMode,
                    LADDR_UNION *dstADDR,
                    BYTE dstEP,
                    BYTE cluster,
                    BYTE srcEP,
                    BYTE* pload,
                    BYTE  plen,
                    BYTE  tsn,
                    BYTE  reqack)
{

    BYTE buf[8];
    BYTE i;

    while(apsBusy()) apsFSM();
    a_aps_service.cmd = LRWPAN_SVC_APS_GENERIC_TX;
    a_aps_tx_data.flags.val = 0;
    a_aps_tx_data.srcEP = srcEP;
    a_aps_tx_data.usrPlen = plen;
    a_aps_tx_data.usrPload = pload;
    a_aps_tx_data.tsn = tsn;
    a_aps_tx_data.cluster = cluster;
    a_aps_tx_data.dstMode = dstMode;
    a_aps_tx_data.dstEP = dstEP;
    a_aps_tx_data.srcSADDR = macGetShortAddr();
    a_aps_tx_data.af_fcf = (1 | AF_FRM_TYPE_MSG);


    if (dstMode == APS_DSTMODE_NONE) {
        a_aps_tx_data.aps_fcf = APS_FRM_TYPE_DATA | APS_FRM_DLVRMODE_INDIRECT;
    } else  {
        a_aps_tx_data.aps_fcf = APS_FRM_TYPE_DATA | APS_FRM_DLVRMODE_NORMAL;
        if (a_aps_tx_data.dstMode == APS_DSTMODE_SHORT ) {
            a_aps_tx_data.dstSADDR = dstADDR->saddr;
            if (dstADDR->saddr == macGetShortAddr()) {
                //sending to ourselves
                a_aps_tx_data.flags.bits.loopback = 1;
            }
        } else {
            a_aps_tx_data.dstLADDR = &dstADDR->laddr.bytes[0];
            a_aps_tx_data.dstSADDR = LRWPAN_SADDR_USE_LADDR;
            /* APS acks to no make sense for LONG addressing, which is
            being used to bypass routing. The MAC acks serve for APS acks
            in this case. Also APS acks would be returned via routing,
            negating the bypass of the routing in the first place.
            */
            reqack = 0;
            //see if we are sending to ourself
            halGetProcessorIEEEAddress(buf);
            for(i = 0; i < 8; i++) {
                if (buf[i] != dstADDR->laddr.bytes[i]) break;
            }
            if (i == 8) {
                //this is a loopback, so use our short address in this field
                a_aps_tx_data.dstSADDR = macGetShortAddr();
                a_aps_tx_data.flags.bits.loopback = 1;
            }
        }
    }
    if (reqack) a_aps_tx_data.aps_fcf |= (APS_FRM_ACKREQ_MASK);

}

//TODO - add a pib flag that indicates when the User's TX
//buffer has been copied so that user can begin building a
//new TX packet while the old one is being TX'ed.



void apsFSM(void)
{


#ifdef LRWPAN_COORDINATOR
    APS_RX_DATA *rxPtr;
#endif



    nwkFSM();
    //if TxFSM is busy we need to call it
    if (apsTXBusy()) 
		apsTxFSM();
    apsRxFSM(); //check RX
    zepFSM();   //do ZEP

apsFSM_start:


    switch (apsState) {
        case APS_STATE_IDLE:

            //ackSendPending check must come before the indirect Pending check!
            if (aps_pib.flags.bits.ackSendPending) {
                apsState = APS_STATE_ACK_SEND_START;
                goto apsFSM_start;
            }

#ifdef LRWPAN_COORDINATOR
            if (aps_pib.flags.bits.indirectPending) {
                //free up RX FSM so that we can receive possible APS acks for this packet.
                aps_pib.flags.bits.indirectPending = 0;  //free up RX FSM
                // an indirect message needs resolving
                //initialize the binding table iterator
                //get pointer to current indirect packet
                rxPtr = &aps_pib.rxBuff[aps_pib.rxTail];
                if (!evbBindTableIterInit(rxPtr->srcEP, rxPtr->srcSADDR, rxPtr->cluster)) {
                    //a failure to initialize the iterator, give up.
                    DEBUG_STRING(DBG_ERR, "APS: BindTable Iterator initialize failed. Discarding indirect msg.\n");
                    apsState = APS_STATE_IDLE;
                } else {
                    apsState = APS_STATE_INDIRECT_GETDST;
                    goto apsFSM_start;
                }
            }
#endif


            break;
        case APS_STATE_COMMAND_START:
            switch(a_aps_service.cmd) {
                case LRWPAN_SVC_APS_GENERIC_TX:
                    aps_pib.flags.bits.IsUsrBufferFree = 0;
#ifdef LRWPAN_COORDINATOR
                    if (APS_GET_FRM_DLVRMODE(a_aps_tx_data.aps_fcf) == APS_FRM_DLVRMODE_INDIRECT) {
                        //this is a special case. A Coordinator endpoint is sending indirect.
                        //we need to inject this into the stack as if it has been received
                        apsState = APS_STATE_INJECT_INDIRECT;
                        goto apsFSM_start;
                    }
#endif


                    if (a_aps_tx_data.flags.bits.loopback) {
                        apsState = APS_STATE_INJECT_LOOPBACK;  //we are sending a direct packet to ourself!
                        goto apsFSM_start;
                    }

                    //Indirect vs Direct is handled within the apsTxData() function
                    //at this point ready to begin formating packet.
                    //do not exit this state until we can grab the TX buffer.
                    if (phyTxUnLocked()) {
                        phyGrabTxLock(); //prevents lower stack layers from grabbing TX buffer.
                        phy_pib.currentTxFlen = 0;  //set frame length to zero, build from scratch
                        apsTxData(TRUE);
                        aps_pib.flags.bits.IsUsrBufferFree = 1;
                        apsState = APS_STATE_GENERIC_TX_WAIT;
                    }
                    break;

                case LRWPAN_SVC_APS_DO_ZEP_TX:
                    if (phyTxUnLocked()) {
                        phyGrabTxLock(); //prevents lower stack layers from grabbing TX buffer.
                        switch(a_aps_service.args.zep_tx.clusterID) {
                            case ZEP_END_DEVICE_ANNOUNCE:
                                zepFmtEndDeviceAnnounce(a_aps_service.args.zep_tx.dst);
                                break;
                            case ZEP_EXTENDED_CMD:
                                switch(a_aps_service.args.zep_tx.extID) {
                                    case ZEP_EXT_NODE_INFO_RSP:
                                        zepFmtNodeInfoRsp(a_aps_service.args.zep_tx.dst);
                                        break;
                                    case ZEP_EXT_PING:
                                        zepFmtPing(a_aps_service.args.zep_tx.dst);
                                        break;
                                    case ZEP_EXT_SEND_ALARM:
                                        zepFmtAlarm(a_aps_service.args.zep_tx.dst,
                                                    a_aps_service.args.zep_tx.ext.alarm.mode
                                                   );
                                        break;

                                }
                                break;
                            default:
                                break;
                        }

                        phy_pib.currentTxFlen = 0;  //set frame length to zero, build from scratch
                        apsTxData(FALSE);           //payload already in TX buffer
                        apsState = APS_STATE_GENERIC_TX_WAIT;
                    }

                    break;


                case LRWPAN_SVC_APS_NWK_PASSTHRU:
                    //for NWK calls that have to pass thru the APS layer
                    //this just serves to lock the APS layer while the
                    //the NWK layer is doing its thing
                    if (nwkBusy()) break;  //wait until nwk is idle
                    nwkDoService();
                    apsState = APS_STATE_NWK_PASSTHRU_WAIT;
                    break;

                default:
                    break;
            }//end switch(a_aps_service.cmd)

            break;

        case APS_STATE_ACK_SEND_START:
            if (phyTxLocked()) break;
            //send an ACK
            //lock the TX buffer
            phyGrabTxLock();
            //we are now ready
            apsFormatAck();
            phy_pib.currentTxFlen = 0;  //set frame length to zero, build from scratch
            apsTxData(TRUE);
            //data sent, release the RX buffer, will let RX FSM resume
            aps_pib.flags.bits.ackSendPending = 0;
            apsState = APS_STATE_GENERIC_TX_WAIT;

            break;

        case APS_STATE_GENERIC_TX_WAIT:
            if (!apsTXIdle()) break;
            //TX is finished, copy status
            a_aps_service.status = apsTxFSM_status;
            //release the TX buffer lock before exiting.
            phyReleaseTxLock();
            apsState = APS_STATE_IDLE;
            if (aps_pib.flags.bits.indirectPending) {
                //have used this state to wait for finishing sending an
                //ACK back to the source of an indirect transmit. Now
                //finish resolving the indirect
                goto apsFSM_start;
            }


            break;

        case APS_STATE_NWK_PASSTHRU_WAIT:
            //for split-phase passthrus
            if (nwkBusy()) break;
            a_aps_service.status = a_nwk_service.status;
            apsState = APS_STATE_IDLE;
            break;

        case APS_STATE_INJECT_LOOPBACK:
            //wait for RX to become idle
            if (apsRxState != APS_RXSTATE_IDLE) break;
            //inject packet into RX FSM
            apsInjectPacket(FALSE);
            aps_pib.flags.bits.IsUsrBufferFree = 1;
            apsState = APS_STATE_IDLE;
            goto apsFSM_start;

#ifdef LRWPAN_COORDINATOR
        case APS_STATE_INJECT_INDIRECT:
            //wait for RX to become idle
            if (apsRxState != APS_RXSTATE_IDLE) break;
            //inject packet into RX FSM
            if (apsRxBuffFull()) {
                //will not be able to copy buffer into indirect space
                a_aps_service.status = LRWPAN_STATUS_INDIRECT_BUFFER_FULL;
            } else {
                apsInjectPacket(TRUE);
            }
            aps_pib.flags.bits.IsUsrBufferFree = 1;
            apsState = APS_STATE_IDLE;
            goto apsFSM_start;

#endif

#ifdef LRWPAN_COORDINATOR
        case APS_STATE_INDIRECT_GETDST:
            rxPtr = &aps_pib.rxBuff[aps_pib.rxTail];
            //get the next bind destination for this src endpoint, cluster
            if (!evbResolveBind(&rxPtr->dstEP, &rxPtr->dstSADDR)) {
                //at this point, we have finished with the indirect transmit.
                //lets free the original packet, and continue
                apsFreeRxPacket(TRUE);
                apsState = APS_STATE_IDLE;
            } else {
                //we have destination
                //see if we are sending to ourself
                if (rxPtr->dstSADDR == macGetShortAddr()) {
                    apsState = APS_STATE_INDIRECT_LOOPBACK;
                } else {
                    apsState = APS_STATE_INDIRECT_TX;
                }
                goto apsFSM_start;
            }
            break;

        case APS_STATE_INDIRECT_LOOPBACK:
            if (apsRxBusy()) break;  //wait until RX buffer free
            //copy to RX buffer
            rxPtr = &aps_pib.rxBuff[aps_pib.rxTail];
            halUtilMemCopy((BYTE *)&a_aps_rx_data, (BYTE *)rxPtr, sizeof(APS_RX_DATA));
            //do user callback
            usrRxPacketCallback();
            apsState = APS_STATE_INDIRECT_GETDST;
            goto apsFSM_start;


        case APS_STATE_INDIRECT_TX:
            //have a destination for our indirect TX. Lets do it.
            if (phyTxLocked()) break;
            phyGrabTxLock();
            rxPtr = &aps_pib.rxBuff[aps_pib.rxTail];
            a_aps_tx_data.aps_fcf = rxPtr->aps_fcf;
            a_aps_tx_data.tsn = rxPtr->tsn;
            a_aps_tx_data.af_fcf = rxPtr->af_fcf;
            a_aps_tx_data.dstEP = rxPtr->dstEP;
            a_aps_tx_data.dstMode = APS_DSTMODE_SHORT;
            a_aps_tx_data.dstSADDR = rxPtr->dstSADDR;
            a_aps_tx_data.cluster = rxPtr->cluster;
            a_aps_tx_data.usrPlen = rxPtr->usrPlen;
            a_aps_tx_data.usrPload = rxPtr->usrPload;
            phy_pib.currentTxFlen = 0;  //set frame length to zero, build from scratch
            apsTxData(TRUE);
            apsState = APS_STATE_INDIRECT_TX_WAIT;
            break;

        //wait for last indirect TX to finish
        case APS_STATE_INDIRECT_TX_WAIT:
            if (nwkBusy()) break;
            //at this point we have a status, but can't do much about it.
            //either it went through or it did not.
            //release the TX buffer lock before exiting.
            phyReleaseTxLock();
            //loop around, and see if there is another destination for this indirect packet
            apsState = APS_STATE_INDIRECT_GETDST;
            goto apsFSM_start;

#endif


        default:
            break;


    }//end switch(apsState)
    HAL_SUSPEND(0);  //for WIN32
}


//inject this packet into stack as if it has been received
//so that the binding can be resolved.
static void apsInjectPacket(BOOL indirect_flag)
{
    BYTE *dst;

    //allocate some heap space for this data
    if (a_aps_tx_data.usrPlen) {
        a_aps_rx_data.orgpkt.data = MemAlloc(a_aps_tx_data.usrPlen);
        if (!a_aps_rx_data.orgpkt.data ) {
            //can't even get started, return
            a_aps_service.status = LRWPAN_STATUS_HEAPFULL;
            return;
        }
    }
    //copy user payload into new space
    dst = a_aps_rx_data.orgpkt.data;
    a_aps_rx_data.usrPlen = a_aps_tx_data.usrPlen;  //save len
    while(a_aps_tx_data.usrPlen) {
        *dst = *a_aps_tx_data.usrPload;  //copy data
        a_aps_tx_data.usrPload++;
        dst++;
        a_aps_tx_data.usrPlen--;
    }
    //set up rest of rx data
    a_aps_rx_data.cluster = a_aps_tx_data.cluster;
    a_aps_rx_data.af_fcf = a_aps_tx_data.af_fcf;
    a_aps_rx_data.srcEP = a_aps_tx_data.srcEP;
    a_aps_rx_data.srcSADDR = a_aps_tx_data.srcSADDR;
    a_aps_rx_data.usrPload = a_aps_rx_data.orgpkt.data;
    a_aps_rx_data.dstEP = a_aps_tx_data.dstEP;
    a_aps_rx_data.orgpkt.rssi = 0xFF;  //highest value


    if (indirect_flag) {
#ifdef LRWPAN_COORDINATOR
        //this packet has arrived at the coordinator
        a_aps_rx_data.dstSADDR = 0;
        //ensure that the submode bit is a '1'
        a_aps_rx_data.aps_fcf = a_aps_tx_data.aps_fcf | APS_FRM_INDIRECT_SUBMODE_MASK ;

        //copy data into indirect buffer space
        apsRxBuffAdd(&a_aps_rx_data);

        //set the RX FSM to the pending state
        aps_pib.flags.bits.indirectPending = 1;
        apsRxState = APS_RXSTATE_RESOLVE_INDIRECT;

#endif
        //at this point, we have simulated this packet being
        //received by the stack. When we return, the mainFSM
        //will process it, resolve the indirect binding, and re-transmit
    } else {
        //this is a direct packet sent to ourselves
        //check for DST endpoint of 0, special endpoint
        if (a_aps_rx_data.dstEP == 0) {
            //not a user endpoint, handle this
            DEBUG_STRING(DBG_INFO, "APS: Received ZEP Request.\n");
            zepHandleRxPacket();
            MemFree(a_aps_rx_data.orgpkt.data);
        } else {
            //deliver to user endpoint right here.
            usrRxPacketCallback();
            //finished, free the space
            MemFree(a_aps_rx_data.orgpkt.data);
        }

    }
    a_aps_service.status = LRWPAN_STATUS_SUCCESS;

}



//Add the AF and APS headers, then send it to NWK
//the AF header probably should be a seperate layer,
//but will place it here since we are only handling MSG frames,
//reduces the depth of the stack.

void apsTxData(BOOL copy_payload)
{

    BYTE *src;

    //if currentTxFlen is zero, we need to build the frame, else, it is
    // a retransmission
    if (phy_pib.currentTxFlen == 0) {
        //assume that the frame is just now being built.
        //use temporary space for building frame
        if (copy_payload) {
            //copy user payload into tmpTxBuff space
            //if userPlen is 0, nothing is copied into the payload area
            phy_pib.currentTxFrm = &tmpTxBuff[LRWPAN_MAX_FRAME_SIZE];
            //get a pointer to the end of the payload
            src = a_aps_tx_data.usrPload + a_aps_tx_data.usrPlen;
            phy_pib.currentTxFlen = a_aps_tx_data.usrPlen;
            //now copy the user payload to the frame
            while (phy_pib.currentTxFlen) {
                src--;                //decrement to first src location with data
                phy_pib.currentTxFrm--;     //decrement to free location
                phy_pib.currentTxFlen--;    //decrement length
                *(phy_pib.currentTxFrm) = *src;
            }
        } else {
            //assume that TXBuff already has the payload, the ZEP
            //commands build their payload in this space
            //point currentTxFrm to this payload
            phy_pib.currentTxFrm = &tmpTxBuff[LRWPAN_MAX_FRAME_SIZE] - a_aps_tx_data.usrPlen;
        }
        //restore length
        phy_pib.currentTxFlen = a_aps_tx_data.usrPlen;

        if (APS_IS_DATA(a_aps_tx_data.aps_fcf)) {
            //DATA frame
            //Build AF header.
            //ONLY MSG FRAMES ARE SUPPORTED, so all we need to write is the
            //length of user payload
            --phy_pib.currentTxFrm;
            phy_pib.currentTxFlen++;
            *phy_pib.currentTxFrm = a_aps_tx_data.usrPlen;

            //sequence number
            --phy_pib.currentTxFrm;
            phy_pib.currentTxFlen++;
            *phy_pib.currentTxFrm = a_aps_tx_data.tsn;

            //AF frame control
            --phy_pib.currentTxFrm;
            phy_pib.currentTxFlen++;
            *phy_pib.currentTxFrm = a_aps_tx_data.af_fcf;
        }


        if (APS_GET_FRM_DLVRMODE(a_aps_tx_data.aps_fcf) == APS_FRM_DLVRMODE_INDIRECT) {
            //this is indirect packet
#ifdef LRWPAN_COORDINATOR
            //TX packet from coordinator, ensure that the submode bit is a '0'

            a_aps_tx_data.aps_fcf = a_aps_tx_data.aps_fcf & ~APS_FRM_INDIRECT_SUBMODE_MASK ;
            //the dstSADDR has already been filled in during the binding resolution, copy to nwk
            a_nwk_tx_data.dstSADDR = a_aps_tx_data.dstSADDR;

#else
            //the destination for indirect packets is the coordinator
            a_nwk_tx_data.dstSADDR = 0;
            //ensure that the submode bit is a '1'
            a_aps_tx_data.aps_fcf = a_aps_tx_data.aps_fcf | APS_FRM_INDIRECT_SUBMODE_MASK ;
#endif
        } else {
            //copy destination address

            a_nwk_tx_data.dstSADDR = a_aps_tx_data.dstSADDR;
            a_nwk_tx_data.dstLADDR = a_aps_tx_data.dstLADDR;
        }



        //Build APS header.
        //SRC Endpoint
        if (!(((APS_GET_FRM_DLVRMODE(a_aps_tx_data.aps_fcf)) == APS_FRM_DLVRMODE_INDIRECT) &&
              (!APS_GET_FRM_INDIRECT_SUBMODE(a_aps_tx_data.aps_fcf)))) {
            //SRC endpoint is only omitted if INDIRECT frame and
            //indirect sub-mode bit is a '0'
            --phy_pib.currentTxFrm;
            phy_pib.currentTxFlen++;
            *phy_pib.currentTxFrm = a_aps_tx_data.srcEP;
        }

        //profile ID
        if ((APS_GET_FRM_TYPE(a_aps_tx_data.aps_fcf) == APS_FRM_TYPE_DATA) ||
            (APS_GET_FRM_TYPE(a_aps_tx_data.aps_fcf) == APS_FRM_TYPE_ACK)) {
            //insert the profile ID, this hardcoded by the configuration
            --phy_pib.currentTxFrm;
            phy_pib.currentTxFlen++;
            *phy_pib.currentTxFrm = (BYTE) ((LRWPAN_APP_PROFILE) >> 8);
            --phy_pib.currentTxFrm;
            phy_pib.currentTxFlen++;
            *phy_pib.currentTxFrm = 0xFF & LRWPAN_APP_PROFILE ;
        }
        //cluster ID
        if (APS_GET_FRM_TYPE(a_aps_tx_data.aps_fcf) == APS_FRM_TYPE_DATA) {
            --phy_pib.currentTxFrm;
            phy_pib.currentTxFlen++;
            *phy_pib.currentTxFrm = a_aps_tx_data.cluster;
        }


        //Destination EP
        if (!(((APS_GET_FRM_DLVRMODE(a_aps_tx_data.aps_fcf)) == APS_FRM_DLVRMODE_INDIRECT) &&
              (APS_GET_FRM_INDIRECT_SUBMODE(a_aps_tx_data.aps_fcf)))) {
            //DST endpoint is only omitted if INDIRECT frame and
            //indirect sub-mode bit is a '1'
            --phy_pib.currentTxFrm;
            phy_pib.currentTxFlen++;
            *phy_pib.currentTxFrm = a_aps_tx_data.dstEP;
        }

        //frame control
        --phy_pib.currentTxFrm;
        phy_pib.currentTxFlen++;
        *phy_pib.currentTxFrm = a_aps_tx_data.aps_fcf;

        if (a_aps_tx_data.flags.bits.loopback) {
            //Zep commands to ourselves have to go all the way through formatting
            //before we inject them into stack
            apsInjectTxPacket();

        } else {

            //setup call to network layer
            //use the SRC address passed in by the aps layer
            //will be the SADDR of the originating node for this message
            a_nwk_tx_data.srcSADDR = a_aps_tx_data.srcSADDR;

            //now set the network bytes
            //since we are using tree routing, the Route Discovery is always suppressed.
            a_nwk_tx_data.radius = LRWPAN_NWK_MAX_RADIUS;
            a_nwk_tx_data.fcflsb = NWK_FRM_TYPE_DATA | NWK_PROTOCOL | NWK_SUPPRESS_ROUTE_DISCOVER ;

            //Send via the network layer
            a_nwk_service.cmd = LRWPAN_SVC_NWK_GENERIC_TX;

            // at this point, we will attempt a TX
            if (APS_GET_FRM_ACKREQ(a_aps_tx_data.aps_fcf)) {
                //need an ACK back. set ackPending bit, start timer.
                aps_pib.flags.bits.ackPending = 1;
                aps_pib.tx_start_time = halGetMACTimer();
                //lets compute our Ack Wait duration
                //aps_pib.apscAckWaitDuration
                aps_pib.apsAckWaitMultiplier = nwkGetHopsToDest(a_nwk_tx_data.dstSADDR);
                aps_pib.apsAckWaitMultiplierCntr = aps_pib.apsAckWaitMultiplier;
            } else aps_pib.flags.bits.ackPending = 0;
            apsSetTxBusy();
            aps_pib.currentAckRetries = aps_pib.apscMaxFrameRetries; //set retry count
            apsTxFSM_status = LRWPAN_STATUS_APS_INPROGRESS;

            //we need to remember this offset in case of a retry, as we
            //will have to reset the flen to this point
            a_aps_tx_data.aps_flen = phy_pib.currentTxFlen;
            a_aps_tx_data.aps_ptr = phy_pib.currentTxFrm;
        }

    }

    if (!a_aps_tx_data.flags.bits.loopback)   nwkDoService();

}

//inject this packet into stack as if it has been received
//so that the binding can be resolved.
static void apsInjectTxPacket(void)
{

    //allocate some heap space for this data
    a_aps_rx_data.orgpkt.data = MemAlloc(phy_pib.currentTxFlen);
    a_aps_rx_data.apsOffset = 0;
    if (!a_aps_rx_data.orgpkt.data ) {
        //can't even get started, return
        //we use th apsTxFSM status to return this status since we are injecting from TX machine
        apsTxFSM_status = LRWPAN_STATUS_HEAPFULL;
        return;
    }
    //copy tx buffer into rx space
    halUtilMemCopy(a_aps_rx_data.orgpkt.data, phy_pib.currentTxFrm, phy_pib.currentTxFlen);
    //set up FCF, RSSI, everything else will be parsed from packet
    a_aps_rx_data.aps_fcf = a_aps_tx_data.aps_fcf;
    //insure that APS field ACK field is NOT set since we will not be getting an ACK back
    a_aps_rx_data.aps_fcf &= ~APS_FRM_ACKREQ_MASK;
    *(a_aps_rx_data.orgpkt.data) = a_aps_rx_data.aps_fcf;

    a_aps_rx_data.orgpkt.rssi = 0xFF;  //highest value since loopback
    a_aps_rx_data.flags.val = 0;
    apsRxState = APS_RXSTATE_START; //kick off RX FSM
    apsRxFSM(); //call apsRxFSM to process packet
    apsTxFSM_status = LRWPAN_STATUS_SUCCESS;
}



//handle RX of packets at APS level
static void apsRxFSM(void)
{
    LRWPAN_STATUS_ENUM callback_status;


apsRxFSM_start:

    switch(apsRxState) {
        case APS_RXSTATE_IDLE:
            break;
        case APS_RXSTATE_START:
            //we have a packet, lets check it out.

            if (APS_IS_RSV(a_aps_rx_data.aps_fcf)) {
                //unknown packet type
                DEBUG_STRING(DBG_INFO, "APS: Received APS RSV packet, discarding.\n");
                MemFree(a_aps_rx_data.orgpkt.data);
                apsRxState = APS_RXSTATE_IDLE;
                break;
            }
            if ((APS_GET_FRM_DLVRMODE(a_aps_rx_data.aps_fcf) == APS_FRM_DLVRMODE_BCAST) ||
                (APS_GET_FRM_DLVRMODE(a_aps_rx_data.aps_fcf) == APS_FRM_DLVRMODE_RSV)) {
                //Delivery mode not handled.
                DEBUG_STRING(DBG_INFO, "APS: Received APS packet with BCAST or RSV delivery mode, discarding.\n");
                MemFree(a_aps_rx_data.orgpkt.data);
                apsRxState = APS_RXSTATE_IDLE;
                break;
            }

            //parse this and see what to do with it.
            if (APS_IS_CMD(a_aps_rx_data.aps_fcf)) {
                //currently don't handle CMD packets. Discard.
                DEBUG_STRING(DBG_INFO, "APS: Received APS CMD packet, discarding.\n");
                //NWK,MAC resource already free; need to free the MEM resource
                MemFree(a_aps_rx_data.orgpkt.data);
                apsRxState = APS_RXSTATE_IDLE;
                break;
            }
            //have a DATA or ACK packet, lets do something.
            //parse it to figure out what it is.
            apsParseHdr(a_aps_rx_data.orgpkt.data + a_aps_rx_data.apsOffset);
            if (APS_IS_ACK(a_aps_rx_data.aps_fcf)) {
                if (!aps_pib.flags.bits.ackPending) {
                    //not currently expecting an ACK so discard
                    DEBUG_STRING(DBG_INFO, "APS: Received unexpected ACK, discarding.\n");
                } else {
                    //lets see if this is our ack.
                    if (apsCheckAck()) {
                        DEBUG_STRING(DBG_INFO, "APS: Received APS ack\n");
                        //this is our ACK, clear the ackPending bit
                        aps_pib.flags.bits.ackPending = 0;
                    } else {
                        DEBUG_STRING(DBG_INFO, "APS: Received ACK, did not match expected.\n");
                    }

                }
                //NWK,MAC resource already free; need to free the MEM resource
                MemFree(a_aps_rx_data.orgpkt.data);
                apsRxState = APS_RXSTATE_IDLE;
                break;
            }

            //check for indirect message
            if (!a_aps_rx_data.flags.bits.dstEP) {
                //no dest endpoint, must be an indirect message.
#ifdef LRWPAN_COORDINATOR
                //see if we have room to buffer this packet
                if (apsRxBuffFull()) {
                    //no room, discard this buffer
                    DEBUG_STRING(DBG_INFO, "APS: No room for indirect packet storage, discarding.\n");
                    MemFree(a_aps_rx_data.orgpkt.data);
                    apsRxState = APS_RXSTATE_IDLE;
                    break;
                }
                //copy this packet to the APS RX buffer
                apsRxBuffAdd (&a_aps_rx_data);
                //first, check to see if an ACK is requested.
                if (APS_GET_FRM_ACKREQ(a_aps_rx_data.aps_fcf)) {
                    //ack request send has to be done in the main FSM
                    aps_pib.flags.bits.ackSendPending = 1;
                    //will have both ackPending and indirectPending set,
                    //the ack pending will be handled first.
                }

                //I am the coordinator, must resolve binding
                aps_pib.flags.bits.indirectPending = 1;
                apsRxState = APS_RXSTATE_RESOLVE_INDIRECT;
                goto apsRxFSM_start;
#else
                //this is an ERROR. Non-coordinator has an unresolved indirect packet.
                DEBUG_STRING(DBG_INFO, "APS: Non-Coord Received indirect packet, should not happen, check code.\n");
                MemFree(a_aps_rx_data.orgpkt.data);
                apsRxState = APS_RXSTATE_IDLE;
                break;
#endif
            }


            //check for DST endpoint of 0, special endpoint
            if (a_aps_rx_data.dstEP == 0) {
                //not a user endpoint, handle this
                DEBUG_STRING(DBG_INFO, "APS: Received ZEP Request.\n");
                callback_status = zepHandleRxPacket();
                goto apsRxFSM_freepkt;
            }

            //at this point, we have a user endpoint. Check profile
            if (a_aps_rx_data.profile != LRWPAN_APP_PROFILE) {
                //reject, not our profile
                DEBUG_STRING(DBG_INFO, "APS: Received data packet with wrong profile, rejecting.\n");
                MemFree(a_aps_rx_data.orgpkt.data);
                apsRxState = APS_RXSTATE_IDLE;
                break;
            }

            //accept this packet, do Usr Callback
            callback_status = usrRxPacketCallback();

        apsRxFSM_freepkt:
            //finished, free the space
            MemFree(a_aps_rx_data.orgpkt.data);

            //see if an ACK is requested. Only send back the ack if the callback status
            //returned as success!
            if (APS_GET_FRM_ACKREQ(a_aps_rx_data.aps_fcf) && callback_status == LRWPAN_STATUS_SUCCESS) {
                //ack request send has to be done in the main FSM
                aps_pib.flags.bits.ackSendPending = 1;
                apsRxState = APS_RXSTATE_ACK_SEND_WAIT;
                break;
            }

            apsRxState = APS_RXSTATE_IDLE;
            break;


#ifdef LRWPAN_COORDINATOR
        case APS_RXSTATE_RESOLVE_INDIRECT:
            if (aps_pib.flags.bits.indirectPending) break;
            //the main FSM will free up the memory associated with the RX packet

            apsRxState = APS_RXSTATE_IDLE;
            break;
#endif

        case APS_RXSTATE_ACK_SEND_WAIT:
            if (aps_pib.flags.bits.ackSendPending) break;  //waiting for ACK to be sent
            //main FSM is finished, can now proceed with new RX
            apsRxState = APS_RXSTATE_IDLE;
            break;

        default:
            break;
    }



}

//see if this matches expected ack.
static BOOL apsCheckAck(void)
{
    if (a_aps_rx_data.flags.bits.dstEP && a_aps_rx_data.flags.bits.srcEP) {
        //both are present.
        if ((a_aps_rx_data.srcEP == a_aps_tx_data.dstEP) &&
            (a_aps_rx_data.dstEP == a_aps_tx_data.srcEP)) {
            //this is our ack
            return TRUE;
        }
    } else if (a_aps_rx_data.flags.bits.dstEP) {
        //indirect, check just the EP
        if ((a_aps_rx_data.dstEP == a_aps_tx_data.srcEP) ) {
            return TRUE;
        }
    } else if (a_aps_rx_data.flags.bits.srcEP) {
        //indirect, check just the EP
        if ((a_aps_rx_data.srcEP == a_aps_tx_data.dstEP) ) {
            return TRUE;
        }

    }
    //if we reach here, must not be our expected ack
    return(FALSE);
}

//Callback from NWK Layer
//Returns TRUE if aps is still busy with last RX packet.

BOOL apsRxBusy(void)
{
    return(apsRxState != APS_RXSTATE_IDLE);
}

void apsRxHandoff(void)
{
    a_aps_rx_data.orgpkt.data = a_nwk_rx_data.orgpkt.data;
    a_aps_rx_data.orgpkt.rssi = a_nwk_rx_data.orgpkt.rssi;
    a_aps_rx_data.apsOffset = a_nwk_rx_data.nwkOffset + 8;
    a_aps_rx_data.srcSADDR = a_nwk_rx_data.srcSADDR;
    a_aps_rx_data.aps_fcf = *(a_aps_rx_data.orgpkt.data + a_aps_rx_data.apsOffset);//get first byte
    a_aps_rx_data.flags.val = 0;
    apsRxState = APS_RXSTATE_START;

}

//parse the APS header, also the AF header as well
static void apsParseHdr(BYTE *ptr)
{
    BYTE len;

    //get the aps fcf byte
    a_aps_rx_data.aps_fcf = *ptr;
    ptr++;

    len = 1;

    if (!((APS_GET_FRM_DLVRMODE(a_aps_rx_data.aps_fcf) == APS_FRM_DLVRMODE_INDIRECT) &&
          (APS_GET_FRM_INDIRECT_SUBMODE(a_aps_rx_data.aps_fcf)))) {
        //have a destination EP
        a_aps_rx_data.flags.bits.dstEP = 1;
        a_aps_rx_data.dstEP = *ptr;
        ptr++;
        len++;

    }
    //get the cluster ID
    if (APS_GET_FRM_TYPE(a_aps_rx_data.aps_fcf) == APS_FRM_TYPE_DATA) {
        a_aps_rx_data.cluster = *ptr;
        ptr++;
    }

    if ((APS_GET_FRM_TYPE(a_aps_rx_data.aps_fcf) == APS_FRM_TYPE_DATA) ||
        (APS_GET_FRM_TYPE(a_aps_rx_data.aps_fcf) == APS_FRM_TYPE_ACK)
       ) {
        //get the profile ID
        a_aps_rx_data.profile = *ptr;
        ptr++;
        a_aps_rx_data.profile += (((UINT16) * ptr) << 8);
        ptr++;
    }

    len = len + 3;

    //get the SRC EP
    if (!((APS_GET_FRM_DLVRMODE(a_aps_rx_data.aps_fcf) == APS_FRM_DLVRMODE_INDIRECT) &&
          (!APS_GET_FRM_INDIRECT_SUBMODE(a_aps_rx_data.aps_fcf)))) {
        //have a SRC EP
        a_aps_rx_data.flags.bits.srcEP = 1;
        a_aps_rx_data.srcEP = *ptr;
        ptr++;
        len++;
    }

    //parse AF frame, assume MSG type
    a_aps_rx_data.afOffset = len;

    //save the af_fcf byte
    a_aps_rx_data.af_fcf = *ptr;
    ptr++;

    //get the transaction number
    a_aps_rx_data.tsn = *ptr;
    ptr++;

    //get the length of the payload
    a_aps_rx_data.usrPlen = *ptr;
    ptr++;
    //save the pointer to the payload
    a_aps_rx_data.usrPload = ptr;

}

//this does not actually format the ACK, just sets
// the aps_tx fields correctly
static void apsFormatAck(void)
{

    a_aps_tx_data.usrPlen = 0;
    a_aps_tx_data.usrPload = NULL;
    a_aps_tx_data.aps_fcf = APS_FRM_TYPE_ACK;
    a_aps_tx_data.srcSADDR = macGetShortAddr();
    a_aps_tx_data.dstSADDR = a_aps_rx_data.srcSADDR;
    a_aps_tx_data.flags.val = 0;


    DEBUG_STRING(DBG_INFO, "APS:Formatted ack\n");
    if (a_aps_rx_data.flags.bits.dstEP && a_aps_rx_data.flags.bits.srcEP) {
        //both are present, so must be direct delivery mode
        a_aps_tx_data.aps_fcf |= APS_FRM_DLVRMODE_NORMAL;
        a_aps_tx_data.dstEP = a_aps_rx_data.srcEP;
        a_aps_tx_data.srcEP = a_aps_rx_data.dstEP;
        a_aps_tx_data.cluster = a_aps_rx_data.cluster;
    } else if (a_aps_rx_data.flags.bits.dstEP) {
        //indirect packet
        //only dstEP is present, so this ack must be going back to the COORD
        a_aps_tx_data.aps_fcf |= APS_FRM_DLVRMODE_INDIRECT;
        a_aps_tx_data.srcEP = a_aps_rx_data.dstEP;
        //set the submode bit indicating only src EP will be present
        a_aps_tx_data.aps_fcf |= APS_FRM_INDIRECT_SUBMODE_MASK;
    } else if (a_aps_rx_data.flags.bits.srcEP) {
        //indirect packet
        //only srcEP is present, so this ack must be going from COORD to src
        a_aps_tx_data.aps_fcf |= APS_FRM_DLVRMODE_INDIRECT;
        a_aps_tx_data.dstEP = a_aps_rx_data.srcEP;
    }

}

//handles retries for APS packets that require ACKs
static void apsTxFSM(void)
{
    if(!apsTXIdle()) {
        //we are not idle
        if (nwkIdle()) {
            //cannot check anything until NWK is idle
            if (a_nwk_service.status != LRWPAN_STATUS_SUCCESS) {
                //don't bother waiting for ACK, TX did not start correctly
                aps_pib.flags.bits.ackPending = 0;
                apsSetTxIdle();  //mark TX as idle
                apsTxFSM_status = a_nwk_service.status; //return status
            } else if (!aps_pib.flags.bits.ackPending) {
                //either no ACK requested or ACK has been received
                apsSetTxIdle();  //finished successfully, mark as idle
                apsTxFSM_status = LRWPAN_STATUS_SUCCESS;
            }
            //check timeout
            //apscAckWaitDuration在apsInit()中初始化的，为200ms
            else if (halMACTimerNowDelta(aps_pib.tx_start_time) > aps_pib.apscAckWaitDuration) {
                //first, check the apsAckWaitMultiplier
                if(aps_pib.apsAckWaitMultiplierCntr) aps_pib.apsAckWaitMultiplierCntr--;  //this is based on number of hops for the apsAck
                if (aps_pib.apsAckWaitMultiplierCntr) {
                    //reset the timer.
                    aps_pib.tx_start_time = halGetMACTimer();
                } else {
                    DEBUG_STRING(1, "APS: TX retry\n");
                    // ACK timeout
                    aps_pib.currentAckRetries--;
                    if (!aps_pib.currentAckRetries) {
                        //retries are zero. We have failed.
                        apsSetTxIdle();
                        DEBUG_STRING(1, "APS TX Retry exceeded\n");
                    } else {
                        //retry...
                        //must reset the len, frm pointers to the beginning of
                        // the formatted aps header before retry
                        phy_pib.currentTxFlen = a_aps_tx_data.aps_flen;
                        phy_pib.currentTxFrm = a_aps_tx_data.aps_ptr;
                        aps_pib.tx_start_time = halGetMACTimer();  //reset the timer
                        aps_pib.apsAckWaitMultiplierCntr = aps_pib.apsAckWaitMultiplier;
                        apsTxData(TRUE);  //reuse the last packet.
                    }
                }
            }

        }

    }

}

#ifdef LRWPAN_COORDINATOR

void apsRxBuffInit(void);
BOOL apsRxBuffFull(void);
BOOL apsRxBuffEmpty(void);
APS_RX_DATA *apsGetRxPacket(void);
void apsFreeRxPacket(BOOL freemem);
void apsRxBuffAdd (APS_RX_DATA *ptr);

//copies data into RX buffer
void apsRxBuffAdd (APS_RX_DATA *ptr)
{
    if (aps_pib.rxCnt == APS_RXBUFF_SIZE) {
        DEBUG_STRING(DBG_ERR, "APS:Trying to add to full buffer in apsRxBuffAdd\n");
        return;
    }
    halUtilMemCopy((BYTE *)&aps_pib.rxBuff[aps_pib.rxHead], (BYTE *)ptr, sizeof(APS_RX_DATA));
    aps_pib.rxCnt++;
    aps_pib.rxHead++; //head points to next free location
    //wrap index
    if (aps_pib.rxHead == APS_RXBUFF_SIZE) aps_pib.rxHead = 0;
}



void apsRxBuffInit(void)
{
    aps_pib.rxCnt = 0;
    aps_pib.rxTail = 0;
    aps_pib.rxHead = 0;
}

BOOL apsRxBuffFull(void)
{
    return(aps_pib.rxCnt == APS_RXBUFF_SIZE);
}


BOOL apsRxBuffEmpty(void)
{
    return(aps_pib.rxCnt == 0);
}

//this does NOT remove the packet from the buffer
APS_RX_DATA *apsGetRxPacket(void)
{
    return(&aps_pib.rxBuff[aps_pib.rxTail]);
}

//frees the first packet in the buffer.
void apsFreeRxPacket(BOOL freemem)
{
    if (aps_pib.rxCnt == 0) {
        DEBUG_STRING(DBG_ERR, "APS:Trying to free empty buffer in apsFreeRxPacket\n");
        return;
    }
    if (freemem)MemFree(aps_pib.rxBuff[aps_pib.rxTail].orgpkt.data);
    aps_pib.rxCnt--;
    aps_pib.rxTail++;
    if (aps_pib.rxTail == APS_RXBUFF_SIZE) aps_pib.rxTail = 0;

}
#endif

