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



/*
Support functions for PC binding demo
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


#ifdef LRWPAN_COORDINATOR
#ifdef LRWPAN_USE_PC_BIND

#include "pcbind.h"

static ROMCHAR crc8tab[] = {
0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4,0xF3};


#define SERIAL_INPUT_TIMEOUT MSECS_TO_MACTICKS(400)


typedef struct _PC_BIND_ELEM{
	SADDR saddr;
	BYTE  ep;
}PC_BIND_ELEM;

PC_BIND_ELEM binding_cache[LRWPAN_PC_BIND_CACHE_SIZE];

BYTE active_bind_cnt;
BYTE active_bind_index;
BYTE bind_status;




static BYTE pbdSendCmdHdr(UINT16 len);
static void pbdSendPayload(BYTE *buf, UINT16 len, BYTE crc);

static void pbdParseSerialCmd(void);
static void pdbParseAlarmReq(BYTE crc);
static void pdbSendAck(BYTE cmd_being_acked, BYTE status);


typedef enum _PBD_STATE_ENUM {
  PBD_STATE_IDLE,
  PBD_STATE_SEND_ALARM,
  PBD_STATE_ALARM_WAIT
}PBD_STATE_ENUM;


PBD_STATE_ENUM pbdState;

typedef union _PDB_ARGS {
	struct {
		BYTE   mode;
		SADDR  saddr;
	}alarm;
}PBD_ARGS;

PBD_ARGS pdb_args;


void pbdInit(void) {
	pbdState = PBD_STATE_IDLE;
}

void pbdFSM(void){

	switch(pbdState)
	{
	case PBD_STATE_IDLE:
		//check if we have input from PC client
		if (halGetchRdy()) pbdParseSerialCmd();
		break;

	case PBD_STATE_SEND_ALARM:
		//wait until APS level is free
		if (apsBusy()) break;
		aplSendAlarm(pdb_args.alarm.saddr,pdb_args.alarm.mode);
        pbdState = PBD_STATE_ALARM_WAIT;
		break;

	case PBD_STATE_ALARM_WAIT:
		if (apsBusy()) break;
		//finished. Send the status back
		pdbSendAck(SCMD_SEND_ALARM_REQ, (BYTE)aplGetStatus());
        pbdState = PBD_STATE_IDLE;
		break;
	}

}


BYTE pbdGetNextCrc (BYTE crc, BYTE val){
	return(crc8tab[crc ^ val]);
}


void pbdSendSerialCommand(BYTE *buf, UINT16 len){
	BYTE crc, x;

	//send header
	halRawPut(SCMD_HDR_BYTE1);
   	halRawPut(SCMD_HDR_BYTE2);

	crc = 0;
	//send length
	x = len & 0xFF;
    crc = pbdGetNextCrc(crc, x);
	halRawPut(x);
	x = (len >> 8) & 0xFF;
    crc = pbdGetNextCrc(crc, x);
	halRawPut(x);
	//send the payload
	while(len) {
     crc = pbdGetNextCrc(crc, *buf);
	 halRawPut(*buf);
	 buf++;
	 len--;
	}
	//send the CRC
    halRawPut(crc);

}

static BYTE pbdSendCmdHdr(UINT16 len) {
	BYTE crc;
	BYTE x;

	crc = 0;
	halRawPut(SCMD_HDR_BYTE1);
	halRawPut(SCMD_HDR_BYTE2);
	x = len & 0xFF;
	crc = pbdGetNextCrc(crc, x);
    halRawPut(x);
    x = (len>>8) & 0xFF;
	crc = pbdGetNextCrc(crc, x);
    halRawPut(x);
	return(crc);
}

static void pdbSendAck(BYTE cmd_being_acked, BYTE status) {

    BYTE crc,x;

	crc = pbdSendCmdHdr(3);

	x = SCMD_CMD_ACK;
	crc = pbdGetNextCrc(crc, x);
    halRawPut(x);

	x = cmd_being_acked;
	crc = pbdGetNextCrc(crc, x);
    halRawPut(x);

	x = status;
	crc = pbdGetNextCrc(crc, x);
    halRawPut(x);
	//send CRC
    halRawPut(crc);
}

static void pbdSendPayload(BYTE *buf, UINT16 len, BYTE crc)
{
  while (len) {
    crc = pbdGetNextCrc(crc, *buf);
    halRawPut(*buf);
    buf++;
    len--;
  }
  //send CRC
  halRawPut(crc);
 }



//send the contents of a ZEP command to the PC client
//the first byte of 'buf' contains the ZEP cluster ID
//the rest is the ZEP payload
void pbdHandleZepForward(void){
	BYTE crc;
	BYTE plen;

	plen = aplGetRxMsgLen() +1;  //+1 includes the ClusterID put into the packet
	crc = pbdSendCmdHdr(plen+1); //add in the byte to the payload length for SCMD_CMD_ZEP byte
	//send the cmd type
	crc = pbdGetNextCrc(crc, SCMD_CMD_ZEP);
	halRawPut(SCMD_CMD_ZEP);
	//send the cluster ID
    crc = pbdGetNextCrc(crc, aplGetRxCluster());
    halRawPut(aplGetRxCluster());
   	//send the payload
	pbdSendPayload(aplGetRxMsgData(),aplGetRxMsgLen(),crc);
}

//get a byte with a timeout.
BOOL pdbGetchWithTimeOut(BYTE *c)
{
	UINT32 my_timer;

	//if character is rdy, return immediately
	if (halGetchRdy()) {
            *c = halGetch();
			return(TRUE);
	}
	//now do timeout
	my_timer= halGetMACTimer();
	do {
		if (halGetchRdy()) {
			*c = halGetch();
			return(TRUE);
		}
	}while ((halMACTimerNowDelta(my_timer))< SERIAL_INPUT_TIMEOUT);
	*c = 0;  //return reasonable default
    return (FALSE);
}

//read the header from a packet sent by PC client
static BOOL pbdGetCmdHdr(UINT16 *rlen, BYTE *rcrc) {
	BYTE x;
	UINT16 len;
	BYTE crc;

	x = 0;

	pdbGetchWithTimeOut(&x) ;
	if (x != SCMD_HDR_BYTE1) return(FALSE);
	pdbGetchWithTimeOut(&x); //get SCMD_HDR_BYTE2
	if (x != SCMD_HDR_BYTE2) return(FALSE);
	crc = 0;
	pdbGetchWithTimeOut(&x);  //get LSB length
	crc = pbdGetNextCrc(crc, x);
	len = x;
	pdbGetchWithTimeOut(&x);  //get MSB length
	crc = pbdGetNextCrc(crc, x);
	len += (((UINT16) x) << 8);
	*rlen = len;
	*rcrc = crc;
	return (TRUE);
}

static void pdbParseAlarmReq(BYTE crc) {
	BYTE x;

	pdbGetchWithTimeOut(&x);  //get LSB
	crc = pbdGetNextCrc(crc, x);
	pdb_args.alarm.saddr = x;
	pdbGetchWithTimeOut(&x);  //get MSB
	crc = pbdGetNextCrc(crc, x);
	pdb_args.alarm.saddr += (((UINT16) x) << 8);

	//get mode
	pdbGetchWithTimeOut(&x);  //get MSB length
	crc = pbdGetNextCrc(crc, x);

	pdb_args.alarm.mode = x;

    pdbGetchWithTimeOut(&x);  //get CRC

	if (x != crc) {
		 DEBUG_STRING(DBG_ERR,"Incorrect CRC for Alarm request\n");
		 //this request will be ignored since we don't kickoff the FSM
		 return;
	}
	//got a valid alarm request, kickoff the FSM state
	pbdState = PBD_STATE_SEND_ALARM;
 }


//parses a generic serial command from the PC client
static void pbdParseSerialCmd(void){
  BYTE crc;
  BYTE x;
  UINT16 tmp;

  //now, lets wait for the response.
  if (!pbdGetCmdHdr(&tmp, &crc)) {
	  DEBUG_STRING(DBG_ERR,"Serial Command Parsing Failed\n");
	  return;
  }
  //get the command
  pdbGetchWithTimeOut(&x);
  crc = pbdGetNextCrc(crc, x);

  switch(x)
  {
    case SCMD_SEND_ALARM_REQ:
	   pdbParseAlarmReq(crc);
	   break;
    case SCMD_CMD_USER_CALLBACK:
	   usrPbdCallback(tmp,crc);
	   break;
   default:
      DEBUG_STRING(DBG_ERR,"Unknown PC Serial Commandn");
	
  }

}

//Reads bindings from PC client

static BOOL pbdReadBindInitRsp(void) {
  BYTE crc;
  BYTE x;
  UINT16 tmp;
  BYTE cnt;

  //now, lets wait for the response.
  if (!pbdGetCmdHdr(&tmp, &crc)) {
	  DEBUG_STRING(DBG_ERR,"PC Bind Table response hdr failed\n");
	  return FALSE;
  }
  //get the command
  pdbGetchWithTimeOut(&x);
  crc = pbdGetNextCrc(crc, x);
  if (x != SCMD_CMD_BINDINIT_RSP) {
      DEBUG_STRING(DBG_ERR,"PC Bind Table response cmd failed\n");
	  return FALSE;
  }
  //get the status
  pdbGetchWithTimeOut(&x);
  crc = pbdGetNextCrc(crc, x);
  bind_status = x;  //save

  //get the number of bindings
  pdbGetchWithTimeOut(&x);
  crc = pbdGetNextCrc(crc, x);
  active_bind_cnt = x;

  if (!active_bind_cnt) {
     DEBUG_STRING(DBG_ERR,"PC Binding found no binding entries!\n");
  }

  //get the binding entries
  cnt = 0;
  while (cnt != active_bind_cnt) {
	//get the SADDR
	pdbGetchWithTimeOut(&x);
    crc = pbdGetNextCrc(crc, x);
	tmp = x;  //LSB
	pdbGetchWithTimeOut(&x);
    crc = pbdGetNextCrc(crc, x);
	tmp += (((UINT16) x) << 8); //MSB
	//save
    binding_cache[cnt].saddr = tmp;
	//get the endpoint
	pdbGetchWithTimeOut(&x);
    crc = pbdGetNextCrc(crc, x);
	binding_cache[cnt].ep = x;
	cnt++;
  }
  //get the CRC
  pdbGetchWithTimeOut(&x);
  if (x != crc) {
	  //wrong CRC, discard
	  active_bind_cnt = 0;
      DEBUG_STRING(DBG_ERR,"PC Binding response, wrong crc!\n");
	  return(FALSE);
  }
  if (!active_bind_cnt) return(FALSE);
  //at this point, we can return TRUE, we have some bindings
  DEBUG_STRING(DBG_INFO,"Got valid bind response!\n");
  return(TRUE);


}

/*

Request the next set of bindings
 Format of Bindinit Req  (1 byte)
 Cmd (SCMD_CMD_BINDGETNEXT_REQ)
 CRC

 This assumes that SCMD_CMD_BINDINIT_REQ has
 already been made, and the PC could not return
 all of the bindings that it found in one packet.

*/

static BOOL pbdSendBindGetNextReq(void){
  BYTE crc;

  active_bind_cnt = 0;
  active_bind_index = 0;
  //send the SCMD_CMD_BINDGETNXT_REQ
  crc = pbdSendCmdHdr(1);
  //send the cmd type
  crc = pbdGetNextCrc(crc, SCMD_CMD_BINDGETNXT_REQ);
  halRawPut(SCMD_CMD_BINDGETNXT_REQ);
  //send the CRC
  halRawPut(crc);
  //now read the response
  return(pbdReadBindInitRsp());
}
	

//send a serial command to the PC to get binding information
/*

Format of Bindinit Req  (6 bytes)  , CRC not included in payload
 Cmd (SCMD_CMD_BINDINIT_REQ)
 Src Saddr (LSB)
 Src Saddr (MSB)
 SrcEP
 Cluster
 Max binds       --maximum return binds that I can handle
 CRC

## response will be a BindInit Rsp
## length is 3 + Numbinds*
 Cmd (SCMD_CMD_BINDINIT_Rsp)
 Status  (0x00 - all bindings found
          0x01 - more to come, need to try again.)
 NumBinds  (number of bindings being returned, always present)
 dst Saddr (LSB)
 dst Saddr (MSB)
 dst EP
 CRC


*/

BOOL evbBindTableIterInit(BYTE srcEP, SADDR srcSADDR, BYTE cluster){
  BYTE crc;
  BYTE x;

  DEBUG_STRING(DBG_INFO,"Sending Binding Iter request!\n");
  active_bind_cnt = 0;
  active_bind_index = 0;
  //send the SCMD_CMD_BINDINIT_REQ
  crc = pbdSendCmdHdr(6);
  //send the cmd type
  crc = pbdGetNextCrc(crc, SCMD_CMD_BINDINIT_REQ);
  halRawPut(SCMD_CMD_BINDINIT_REQ);
  //send SADDR
  x = srcSADDR & 0xFF;
  crc = pbdGetNextCrc(crc, x);
  halRawPut(x);
  x = ((srcSADDR) >> 8) & 0xFF;
  crc = pbdGetNextCrc(crc, x);
  halRawPut(x);
  //send EP
  crc = pbdGetNextCrc(crc, srcEP);
  halRawPut(srcEP);
  //send Cluster
  crc = pbdGetNextCrc(crc, cluster);
  halRawPut(cluster);
  //send the max number of binds to return
  crc = pbdGetNextCrc(crc, LRWPAN_PC_BIND_CACHE_SIZE);
  halRawPut(LRWPAN_PC_BIND_CACHE_SIZE);
  //send the CRC
  halRawPut(crc);
  return(pbdReadBindInitRsp());
}

//return the next bind that is in the list
BOOL evbResolveBind(BYTE *dstEP, SADDR *dstSADDR){
	if (!active_bind_cnt) return(FALSE);
	if (active_bind_index == active_bind_cnt) {
		//see if PC has more bindings for us
		if (!bind_status) return(FALSE);  //last status was last
		//there are more bindings, request them
		if (!pbdSendBindGetNextReq()) return(FALSE);
		if (!active_bind_cnt) return(FALSE);
		//if we get to here, we have fetched more bindings
	}
      DEBUG_STRING(DBG_INFO,"Next Indirect dest is SADDR: ");
      DEBUG_UINT16(DBG_INFO,binding_cache[active_bind_index].saddr);
      DEBUG_STRING(DBG_INFO,", EP: ");
      DEBUG_UINT8(DBG_INFO,binding_cache[active_bind_index].ep);
      DEBUG_STRING(DBG_INFO,"\n");

	*dstSADDR = binding_cache[active_bind_index].saddr;
	*dstEP = binding_cache[active_bind_index].ep;
    active_bind_index++;
	return(TRUE);
}



#endif
#endif


