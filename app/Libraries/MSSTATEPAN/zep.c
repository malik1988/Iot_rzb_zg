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
Handles messages for the Zero Endpoint

*/

#include "compiler.h"               //compiler specific
#include "lrwpan_common_types.h"
#include "lrwpan_config.h"
#include "ieee_lrwpan_defs.h"
#include "hal.h"
#include "console.h"
#include "debug.h"
#include "phy.h"
#include "mac.h"
#include "nwk.h"
#include "aps.h"
#include "zep.h"
#include "neighbor.h"
#include "evboard.h"

#ifdef LRWPAN_USE_PC_BIND
#include "pcbind.h"
#endif

ZEP_STATE_ENUM zepState;
ZEP_PIB zep_pib;


static void zepCommonFmt(SADDR dst_saddr, BYTE cluster);

//locals
#ifdef LRWPAN_FFD
static LRWPAN_STATUS_ENUM zepHandleEndDeviceAnnounce(void);
#endif



void zepInit(void)
{
    zepState = ZEP_STATE_IDLE;
    zep_pib.flags.val = 0;
}

void zepFSM(void)
{
    //does not do much right now
    static unsigned char ledState = 0;  //¿ØÖÆled×´Ì¬,

    if (zep_pib.flags.bits.Alarm) {
        //flash LED1 at 200ms rate
        if ((halMACTimerNowDelta(zep_pib.alarm_timer)) > MSECS_TO_MACTICKS(200)) {
            //toggle LED1
            if (ledState) {
                EVB_LED1_OFF();
            } else {
                EVB_LED1_ON();
            }
            zep_pib.alarm_timer = halGetMACTimer();
        }


    }

}

//right now, we only handle simple RX packets.
//This must completely process the current packet and post jobs if this
//needs to be handled.
LRWPAN_STATUS_ENUM zepHandleRxPacket(void)
{

    LRWPAN_STATUS_ENUM rstatus;
    BYTE *ptr;

    rstatus = LRWPAN_STATUS_SUCCESS;
    ptr = aplGetRxMsgData();
    switch (a_aps_rx_data.cluster) {
#ifdef LRWPAN_FFD
        case ZEP_END_DEVICE_ANNOUNCE:
            rstatus = zepHandleEndDeviceAnnounce();
            if (rstatus == LRWPAN_STATUS_SUCCESS) {
                rstatus = usrZepRxCallback();
            }
            break;
#endif
        case ZEP_EXTENDED_CMD:
            switch(*ptr) {
                case ZEP_EXT_NODE_INFO_RSP:
#ifdef LRWPAN_COORDINATOR
#ifdef LRWPAN_USE_PC_BIND
                    //forward the ZEP info to the PC client
                    pbdHandleZepForward();
#endif
#endif
                    rstatus = usrZepRxCallback();
                    break;
                case ZEP_EXT_PING:
#ifdef LRWPAN_COORDINATOR
#ifdef LRWPAN_USE_PC_BIND
                    //forward the ping to the PC client
                    pbdHandleZepForward();
#endif
#endif
                    rstatus = LRWPAN_STATUS_SUCCESS;
                    break;

                case ZEP_EXT_SEND_ALARM:
                    ptr++;
                    //check the mode
                    if (*ptr) {
                        //turn on the alarm
                        zep_pib.flags.bits.Alarm = 1;
                        zep_pib.alarm_timer = halGetMACTimer();
                        if (EVB_LED1_STATE()) {
                            EVB_LED1_OFF();
                        } else {
                            EVB_LED1_ON();
                        }
                    } else {
                        //turn off the alarm
                        zep_pib.flags.bits.Alarm = 0;
                        //turn off the LED
                        EVB_LED1_OFF();
                    }


                    break;

                default:
                    break;

            }
            break;
        default:
            DEBUG_STRING(DBG_INFO, "Unhandled Zero Endpoint Command, discarding.\n");
    }
    return(rstatus);
}

static void zepCommonFmt(SADDR dst_saddr, BYTE cluster)
{
    a_aps_tx_data.flags.val = 0;
    a_aps_tx_data.srcEP = 0;    //from endpoint 0
    a_aps_tx_data.tsn = apsGenTSN();
    a_aps_tx_data.cluster = cluster;
    a_aps_tx_data.dstMode = APS_DSTMODE_SHORT;
    a_aps_tx_data.dstSADDR = dst_saddr;  //destination
    a_aps_tx_data.dstEP = 0;     //to endpoint 0
    a_aps_tx_data.srcSADDR = macGetShortAddr();
    a_aps_tx_data.af_fcf = (1 | AF_FRM_TYPE_MSG);
    //use an ACK so that we know if this succeeds
    a_aps_tx_data.aps_fcf = APS_FRM_TYPE_DATA | APS_FRM_DLVRMODE_NORMAL | APS_FRM_ACKREQ_MASK;
    //set the loopback flag if this is for us, have to check this for every ZEP format
    if (a_aps_tx_data.dstSADDR == macGetShortAddr()) a_aps_tx_data.flags.bits.loopback = 1;

}

//send a payload that has our short address, so that a ping
//can be used to send our short address to somebody
//The main goal is to the APS ACK back
//as verification that this was delivered
//this ping packet is also forwarded to the PC client
//if PC binding is being used.

void zepFmtPing(SADDR dst_saddr)
{
    BYTE *ptr;
    a_aps_tx_data.usrPlen = 3;
    ptr = &tmpTxBuff[LRWPAN_MAX_FRAME_SIZE - 1];
    *ptr = (BYTE) (macGetShortAddr() >> 8);
    --ptr;
    *ptr = (BYTE) (macGetShortAddr());
    --ptr;
    *ptr = ZEP_EXT_PING;
    zepCommonFmt(dst_saddr, ZEP_EXTENDED_CMD);
}

//turns an alarm on at the node
//if mode is nonzero, turn on the alarm
//if mode is zero, turn off the alarm
void zepFmtAlarm(SADDR dst_saddr, BYTE mode)
{
    BYTE *ptr;
    a_aps_tx_data.usrPlen = 2;
    ptr = &tmpTxBuff[LRWPAN_MAX_FRAME_SIZE - 1];
    *ptr = mode;
    --ptr;
    *ptr = ZEP_EXT_SEND_ALARM;
    zepCommonFmt(dst_saddr, ZEP_EXTENDED_CMD);
}


#ifdef LRWPAN_FFD
//an End Device has sent us its short address, long address information.
//put this in the address map.
static LRWPAN_STATUS_ENUM zepHandleEndDeviceAnnounce(void)
{
    BYTE *ptr;
    SADDR saddr;

    if (aplGetRxMsgLen() != ZEP_PLEN_END_DEVICE_ANNOUNCE)  return (LRPAN_STATUS_ZEP_FAILED) ;  // wrong message length.
    ptr = aplGetRxMsgData();  //get pointer to data
    //parse the message.
    saddr =  *ptr;
    ptr++;
    saddr += (((UINT16) * ptr) << 8);
    ptr++;
    //enter this into the map
    ntNewAddressMapEntry(ptr, saddr);
    return(LRWPAN_STATUS_SUCCESS);
}

#endif



//put our long address, short address into the tmpTxBuffer
//so that it can be sent to the coordinator.
void zepFmtEndDeviceAnnounce(SADDR dst_saddr)
{
    BYTE *ptr;

    //first, do the payload
    ptr = &tmpTxBuff[LRWPAN_MAX_FRAME_SIZE] - 8;
    //copy in the long address
    halGetProcessorIEEEAddress(ptr);
    //now put our short address
    --ptr;
    *ptr = (BYTE) (macGetShortAddr() >> 8);

    --ptr;
    *ptr = (BYTE) (macGetShortAddr());

    a_aps_tx_data.usrPlen = ZEP_PLEN_END_DEVICE_ANNOUNCE;

    //now, the rest
    zepCommonFmt(dst_saddr, ZEP_END_DEVICE_ANNOUNCE);

}

/* this is a custom Zero End point command that sends data that is spread
across several different Zigbee response commands into one. This info
is needed by the PC client in the binding demo
*/
void zepFmtNodeInfoRsp(SADDR dst_saddr)
{

    BYTE *ptr;
    BYTE i;

    //endpoints first
    ptr = &tmpTxBuff[LRWPAN_MAX_FRAME_SIZE];
    //do the endpoints first.
    for (i = 0; i < aps_pib.activeEPs; i++) {
        --ptr;
        *ptr = apsEndPoints[i].epNum;
    }
    --ptr;
    *ptr = aps_pib.activeEPs;

    //MAC capability code
    --ptr;
    *ptr = mac_pib.macCapInfo;

    //Node type
    --ptr;
#if defined(LRWPAN_COORDINATOR)
    *ptr = NODE_TYPE_COORD;
#elif defined(LRWPAN_FFD)
    *ptr = NODE_TYPE_ROUTER;
#else
    *ptr = NODE_TYPE_ENDDEVICE;
#endif
    //my parent
    --ptr;
    *ptr = (mac_pib.macCoordShortAddress >> 8) & 0xFF;


    --ptr;
    *ptr = mac_pib.macCoordShortAddress & 0xFF;


    //my short address
    --ptr;
    *ptr = (macGetShortAddr() >> 8) & 0xFF;


    --ptr;
    *ptr = macGetShortAddr() & 0xFF;

    //copy in the long address
    ptr = ptr - 8;
    halGetProcessorIEEEAddress(ptr);

    //indentify this extended command
    --ptr;
    *ptr = ZEP_EXT_NODE_INFO_RSP;

    a_aps_tx_data.usrPlen = aps_pib.activeEPs + ZEP_PLEN_NODE_INFO_RSP ;

    //now, the rest
    zepCommonFmt(dst_saddr, ZEP_EXTENDED_CMD);

}


