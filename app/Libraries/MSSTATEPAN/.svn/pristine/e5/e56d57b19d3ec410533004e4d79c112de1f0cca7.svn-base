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


#ifndef ZEP_H
#define ZEP_H

//Zero Endpoint Header


#define ZEP_END_DEVICE_ANNOUNCE    0x13
#define ZEP_PLEN_END_DEVICE_ANNOUNCE 10



//undefined end-device commands used by this stack
#define ZEP_EXTENDED_CMD          0xFF
#define ZEP_EXT_PING              0x01
#define ZEP_EXT_NODE_INFO_RSP     0x02
#define ZEP_EXT_SEND_ALARM        0x03

#define ZEP_PLEN_NODE_INFO_RSP     16

typedef enum _ZEP_STATE_ENUM {
  ZEP_STATE_IDLE,
  ZEP_STATE_RX_START
} ZEP_STATE_ENUM;


typedef struct _ZEP_PIB{
	union _ZEP_PIB_FLAGS{
		BYTE val;
		struct {
			unsigned Alarm:1;
		}bits;
	}flags;
	UINT32 alarm_timer;
}ZEP_PIB;

extern ZEP_STATE_ENUM zepState;
void zepInit(void);
void zepFSM(void);
void zepFmtEndDeviceAnnounce(SADDR dst);
void zepFmtNodeInfoRsp(SADDR dst);
void zepFmtPing(SADDR dst);
void zepFmtAlarm(SADDR dst_saddr, BYTE mode);

LRWPAN_STATUS_ENUM zepHandleRxPacket(void);

LRWPAN_STATUS_ENUM usrZepRxCallback(void);

#define zepIdle() (zepState == ZEP_STATE_IDLE)
#define zepBusy() (zepState != ZEP_STATE_IDLE)

#endif


