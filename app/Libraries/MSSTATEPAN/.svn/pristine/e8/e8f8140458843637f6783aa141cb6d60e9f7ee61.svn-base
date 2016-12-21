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


#ifndef IEEE_LRWPAN_DEFS_H
#define IEEE_LRWPAN_DEFS_H
//IEEE 802.15.4 Frame definitions here

#define LRWPAN_BCAST_SADDR       0xFFFF
#define LRWPAN_BCAST_PANID       0xFFFF
#define LRWPAN_SADDR_USE_LADDR   0xFFFE

#define LRWPAN_ACKFRAME_LENGTH 5

#define LRWPAN_MAX_MACHDR_LENGTH 23
#define LRWPAN_MAX_NETHDR_LENGTH 8
#define LRWPAN_MAX_APSHDR_LENGTH 5

#define LRWPAN_MAXHDR_LENGTH (LRWPAN_MAX_MACHDR_LENGTH+LRWPAN_MAX_NETHDR_LENGTH+LRWPAN_MAX_APSHDR_LENGTH)



#define LRWPAN_MAX_FRAME_SIZE 127

#define LRWPAN_FRAME_TYPE_BEACON 0
#define LRWPAN_FRAME_TYPE_DATA 1
#define LRWPAN_FRAME_TYPE_ACK 2
#define LRWPAN_FRAME_TYPE_MAC 3

//BYTE masks
#define LRWPAN_FCF_SECURITY_MASK 0x8
#define LRWPAN_FCF_FRAMEPEND_MASK 0x10
#define LRWPAN_FCF_ACKREQ_MASK 0x20
#define LRWPAN_FCF_INTRAPAN_MASK 0x40



#define LRWPAN_SET_FRAME_TYPE(x,f)     (x=x|f)
#define LRWPAN_GET_FRAME_TYPE(x)     (x&0x03)

#define LRWPAN_SET_SECURITY_ENABLED(x) BITSET(x,3)
#define LRWPAN_SET_FRAME_PENDING(x)    BITSET(x,4)
#define LRWPAN_SET_ACK_REQUEST(x)      BITSET(x,5)
#define LRWPAN_SET_INTRAPAN(x)         BITSET(x,6)

#define LRWPAN_CLR_SECURITY_ENABLED(x) BITCLR(x,3)
#define LRWPAN_CLR_FRAME_PENDING(x)    BITCLR(x,4)
#define LRWPAN_CLR_ACK_REQUEST(x)      BITCLR(x,5)
#define LRWPAN_CLR_INTRAPAN(x)         BITCLR(x,6)

#define LRWPAN_GET_SECURITY_ENABLED(x) BITTST(x,3)
#define LRWPAN_GET_FRAME_PENDING(x)    BITTST(x,4)
#define LRWPAN_GET_ACK_REQUEST(x)      BITTST(x,5)
#define LRWPAN_GET_INTRAPAN(x)         BITTST(x,6)


#define LRWPAN_ADDRMODE_NOADDR 0
#define LRWPAN_ADDRMODE_SADDR  2
#define LRWPAN_ADDRMODE_LADDR  3

#define LRWPAN_GET_DST_ADDR(x) ((x>>2)&0x3)
#define LRWPAN_GET_SRC_ADDR(x) ((x>>6)&0x3)
#define LRWPAN_SET_DST_ADDR(x,f) (x=x|(f<<2))
#define LRWPAN_SET_SRC_ADDR(x,f) (x=x|(f<<6))

#define LRWPAN_FCF_DSTMODE_MASK   (0x03<<2)
#define LRWPAN_FCF_DSTMODE_NOADDR (LRWPAN_ADDRMODE_NOADDR<<2)
#define LRWPAN_FCF_DSTMODE_SADDR (LRWPAN_ADDRMODE_SADDR<<2)
#define LRWPAN_FCF_DSTMODE_LADDR (LRWPAN_ADDRMODE_LADDR<<2)

#define LRWPAN_FCF_SRCMODE_MASK   (0x03<<6)
#define LRWPAN_FCF_SRCMODE_NOADDR (LRWPAN_ADDRMODE_NOADDR<<6)
#define LRWPAN_FCF_SRCMODE_SADDR (LRWPAN_ADDRMODE_SADDR<<6)
#define LRWPAN_FCF_SRCMODE_LADDR (LRWPAN_ADDRMODE_LADDR<<6)

#define LRWPAN_IS_ACK(x) (LRWPAN_GET_FRAME_TYPE(x) == LRWPAN_FRAME_TYPE_ACK)
#define LRWPAN_IS_BCN(x) (LRWPAN_GET_FRAME_TYPE(x) == LRWPAN_FRAME_TYPE_BEACON)
#define LRWPAN_IS_MAC(x) (LRWPAN_GET_FRAME_TYPE(x) == LRWPAN_FRAME_TYPE_MAC)
#define LRWPAN_IS_DATA(x) (LRWPAN_GET_FRAME_TYPE(x) == LRWPAN_FRAME_TYPE_DATA)

//The ASSOC Req and Rsp are not 802 compatible as more information is 
//added to these packets than is in the spec.

#ifdef IEEE_802_COMPLY
#define LRWPAN_MACCMD_ASSOC_REQ       0x01      
#define LRWPAN_MACCMD_ASSOC_RSP       0x02   
#else
#define LRWPAN_MACCMD_ASSOC_REQ       0x81      
#define LRWPAN_MACCMD_ASSOC_RSP       0x82   
#endif

#define LRWPAN_MACCMD_DISASSOC        0x03
#define LRWPAN_MACCMD_DATA_REQ        0x04
#define LRWPAN_MACCMD_PAN_CONFLICT    0x05
#define LRWPAN_MACCMD_ORPHAN          0x06
#define LRWPAN_MACCMD_BCN_REQ         0x07
#define LRWPAN_MACCMD_COORD_REALIGN   0x08
#define LRWPAN_MACCMD_GTS_REQ         0x09

#ifdef IEEE_802_COMPLY
#define LRWPAN_MACCMD_ASSOC_REQ_PAYLOAD_LEN 2    
#define LRWPAN_MACCMD_ASSOC_RSP_PAYLOAD_LEN 4  
#else
#define LRWPAN_MACCMD_ASSOC_REQ_PAYLOAD_LEN 6  //has four extra bytes in it, 'magic number'
#define LRWPAN_MACCMD_ASSOC_RSP_PAYLOAD_LEN 7  //has three extra bytes, shortAddr & depth of parent
#endif

#define LRWPAN_MACCMD_COORD_REALIGN_PAYLOAD_LEN 8


//this is only for our beacons
#ifdef LRWPAN_ZIGBEE_BEACON_COMPLY
#define LRWPAN_NWK_BEACON_SIZE (9+4)    //9 byte payload, 4 byte header    
#else
#define LRWPAN_NWK_BEACON_SIZE (9+4+4) //add in an extra four-byte magic number
#endif



#define LRWPAN_GET_CAPINFO_ALTPAN(x)       BITTST(x,0)
#define LRWPAN_GET_CAPINFO_DEVTYPE(x)      BITTST(x,1)    
#define LRWPAN_GET_CAPINFO_PWRSRC(x)       BITTST(x,2)      
#define LRWPAN_GET_CAPINFO_RONIDLE(x)      BITTST(x,3) 
#define LRWPAN_GET_CAPINFO_SECURITY(x)     BITTST(x,6) 
#define LRWPAN_GET_CAPINFO_ALLOCADDR(x)    BITTST(x,7) 

#define LRWPAN_SET_CAPINFO_ALTPAN(x)       BITSET(x,0)
#define LRWPAN_SET_CAPINFO_DEVTYPE(x)      BITSET(x,1)    
#define LRWPAN_SET_CAPINFO_PWRSRC(x)       BITSET(x,2)      
#define LRWPAN_SET_CAPINFO_RONIDLE(x)      BITSET(x,3) 
#define LRWPAN_SET_CAPINFO_SECURITY(x)     BITSET(x,6) 
#define LRWPAN_SET_CAPINFO_ALLOCADDR(x)    BITSET(x,7) 

#define LRWPAN_CLR_CAPINFO_ALTPAN(x)       BITCLR(x,0)
#define LRWPAN_CLR_CAPINFO_DEVTYPE(x)      BITCLR(x,1)    
#define LRWPAN_CLR_CAPINFO_PWRSRC(x)       BITCLR(x,2)      
#define LRWPAN_CLR_CAPINFO_RONIDLE(x)      BITCLR(x,3) 
#define LRWPAN_CLR_CAPINFO_SECURITY(x)     BITCLR(x,6) 
#define LRWPAN_CLR_CAPINFO_ALLOCADDR(x)    BITCLR(x,7) 

//BEACON defs
#define LRWPAN_BEACON_SF_ASSOC_PERMIT_MASK (1<<7)
#define LRWPAN_BEACON_SF_PAN_COORD_MASK    (1<<6)

#define LRWPAN_GET_BEACON_SF_ASSOC_PERMIT(x) ( (x) & (LRWPAN_BEACON_SF_ASSOC_PERMIT_MASK))


//Association status
#define LRWPAN_ASSOC_STATUS_SUCCESS 0
#define LRWPAN_ASSOC_STATUS_NOROOM  1
#define LRWPAN_ASSOC_STATUS_DENIED  2
#define LRWPAN_ASSOC_STATUS_MASK    3

#define LRWPAN_GET_ASSOC_STATUS(x) ((x)&LRWPAN_ASSOC_STATUS_MASK)





#endif
