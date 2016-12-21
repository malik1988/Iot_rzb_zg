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



// Default static bind resolution functions


#ifndef PCBIND_H
#define PCBIND_H


BYTE pbdGetNextCrc (BYTE crc, BYTE val);
void pbdSendSerialCommand(BYTE *buf, UINT16 len);
void pbdHandleZepForward(void);
void pbdSaveZepCmd(void);
void pbdFSM(void);
void pbdInit(void);


void usrPbdCallback(UINT16 pktlen, BYTE crc);



#define SCMD_HDR_BYTE1  0x01
#define SCMD_HDR_BYTE2  0x02

#define SCMD_CMD_ZEP             0x01
#define SCMD_CMD_BINDINIT_REQ    0x02
#define SCMD_CMD_BINDINIT_RSP    0x82


#define SCMD_CMD_BINDGETNXT_REQ    0x03

#define SCMD_SEND_ALARM_REQ        0x04   

#define SCMD_CMD_ACK               0xFE   //generic ACK
#define SCMD_CMD_USER_CALLBACK     0xFF

#endif




