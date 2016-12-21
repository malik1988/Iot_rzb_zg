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
V0.2.5 Fixed bug in ntFindBySADDR      10/Oct/2006 RBR,  reported by Dan Shone.
V0.2 added PC-based binding         21/July/2006  RBR
V0.1 Initial Release                10/July/2006  RBR

*/


/*
Neighbor Table Support and Address assignment support


*/

#include "compiler.h"
#include "lrwpan_config.h"         //user configurations
#include "lrwpan_common_types.h"   //types common acrosss most files
#include "ieee_lrwpan_defs.h"
#include "console.h"
#include "debug.h"
#include "memalloc.h"
#include "hal.h"
#include "halStack.h"
#include "phy.h"
#include "mac.h"


#include "neighbor.h"

//holds pairs of long/short addresses


#ifdef LRWPAN_FFD
NAYBORENTRY mac_nbr_tbl[NTENTRIES];
#endif


//initializes the Address map
//called in halInit()
//also called anytime the Coord forms a network
//or a router joins a network.
//the first entry in the address map is always our own
//that maps long address to short address
void ntInitAddressMap(void){
  BYTE j;
  for (j=0;j<LRWPAN_MAX_ADDRESS_MAP_ENTRIES;j++) {
		mac_addr_tbl[j].saddr = LRWPAN_BCAST_SADDR;
	}
}
	

//address map functions
//finds a match in the address map table using a LADDR
BOOL ntFindAddressByLADDR(LADDR *ptr, BYTE *index){
   BYTE j,i;
   BYTE *src,*dst;

   for (j=0;j<LRWPAN_MAX_ADDRESS_MAP_ENTRIES;j++) {
		if (mac_addr_tbl[j].saddr == LRWPAN_BCAST_SADDR) continue;
		src = &ptr->bytes[0];
		dst = &mac_addr_tbl[j].laddr[0];
		for (i=0;i<8;i++, src++, dst++) {
			if (*src != *dst) break;
		}
		if (i== 8) {
			//have a match
		    *index = j;
		    break;
		}		
   }

   if (j != LRWPAN_MAX_ADDRESS_MAP_ENTRIES) return(TRUE);
	else return(FALSE);	

}

void ntAddOurselvesToAddressTable(SADDR saddr){
	BYTE laddr[8];

	halGetProcessorIEEEAddress(&laddr[0]);
	ntNewAddressMapEntry(&laddr[0], saddr);
}


//finds a match in the address map table using a SADDR
BOOL ntFindAddressBySADDR(SADDR saddr, BYTE *index)
{
   BYTE j;

   for (j=0;j<LRWPAN_MAX_ADDRESS_MAP_ENTRIES;j++) {
		if (mac_addr_tbl[j].saddr == LRWPAN_BCAST_SADDR) continue;
		if (mac_addr_tbl[j].saddr != saddr) continue;
		*index = j;
		break;
		
	}

   if (j != LRWPAN_MAX_ADDRESS_MAP_ENTRIES) return(TRUE);
	else return(FALSE);

}

//checks if  laddr, saddr is already in the address map table
//if it is, return map index in 'index'
BOOL ntCheckAddressMapEntry(BYTE *laddr, SADDR saddr, BYTE *index) {
   BYTE j,i;
   BYTE *src,*dst;

   for (j=0;j<LRWPAN_MAX_ADDRESS_MAP_ENTRIES;j++) {
		if (mac_addr_tbl[j].saddr == LRWPAN_BCAST_SADDR) continue;
		if (mac_addr_tbl[j].saddr != saddr) continue;
		src = laddr;
		dst = &mac_addr_tbl[j].laddr[0];
		for (i=0;i<8;i++) {
			if (*src != *dst) break;
			src++; dst++;
		}
		if (i == 8) {
			// we have a match
			*index = j;
			return(TRUE);
		}
	}

	return(FALSE);
}

//enters new laddr, saddr into address map entry
SADDR ntNewAddressMapEntry(BYTE *laddr, SADDR saddr) {
    BYTE j;

	if (ntCheckAddressMapEntry(laddr, saddr, &j)) {
		//entry is already in the table.
		return(mac_addr_tbl[j].saddr);
	}
	//now find free entry in address map table
	for (j=0;j<LRWPAN_MAX_ADDRESS_MAP_ENTRIES;j++) {
		if (mac_addr_tbl[j].saddr == LRWPAN_BCAST_SADDR) break;
	}
	if (j== LRWPAN_MAX_ADDRESS_MAP_ENTRIES) return(LRWPAN_BCAST_SADDR);//error, no room
    halUtilMemCopy(&mac_addr_tbl[j].laddr[0], laddr, 8);
	mac_addr_tbl[j].saddr = saddr;
	return(0);
}





UINT16 ntGetCskip(BYTE depth) {
	switch(depth){
		case 1: return(LRWPAN_CSKIP_1);
		case 2: return(LRWPAN_CSKIP_2);
		case 3: return(LRWPAN_CSKIP_3);
		case 4: return(LRWPAN_CSKIP_4);
		case 5: return(LRWPAN_CSKIP_5);
		case 6: return(LRWPAN_CSKIP_6);
		case 7: return(LRWPAN_CSKIP_7);
		case 8: return(LRWPAN_CSKIP_8);
		case 9: return(LRWPAN_CSKIP_9);
		case 10: return(LRWPAN_CSKIP_10);
	}
return(0);
}

SADDR ntGetMaxSADDR(SADDR router_saddr,BYTE depth){
	//compute the maximum SADDR given the router_saddr and depth

   return(router_saddr + (ntGetCskip(depth)*(LRWPAN_MAX_ROUTERS_PER_PARENT))
	      + LRWPAN_MAX_NON_ROUTER_CHILDREN);
}


//rest of this are neighbor table functions, only needed by FFDs

#ifdef LRWPAN_FFD


NAYBORENTRY *ntFindBySADDR (UINT16 saddr){

	NAYBORENTRY *nt_ptr;
	BYTE j;

	nt_ptr = &mac_nbr_tbl[0];
	for (j=0;j<NTENTRIES;j++,nt_ptr++) {
		if ( nt_ptr->flags.bits.used &&
			mac_addr_tbl[nt_ptr->map_index].saddr == saddr) return(nt_ptr);
	  }
	return(NULL);
}

NAYBORENTRY *ntFindByLADDR (LADDR *ptr){
	NAYBORENTRY *nt_ptr;
	BYTE j,i;

    nt_ptr = &mac_nbr_tbl[0];
	for (j=0;j<NTENTRIES;j++,nt_ptr++) {
		if (!nt_ptr->flags.bits.used) continue;
		for (i=0;i<8;i++) {
			if (mac_addr_tbl[nt_ptr->map_index].laddr[i] != ptr->bytes[i]) break;
		}
		if (i == 8)	return(nt_ptr);
	}
	return(NULL);
}



//Init neighbor table. Called when Network is formed by
//coordinator or when a Router successfully joins a network.
//this also initializes the address table map
void ntInitTable(void) {
	NAYBORENTRY *nt_ptr;
	BYTE j;
	
	nt_ptr = &mac_nbr_tbl[0];
	for (j=0;j<NTENTRIES;j++,nt_ptr++) {
		nt_ptr->flags.val = 0;
	}
	ntInitAddressMap();	
}

void ntInitAddressAssignment(void){
    //also initialize ADDRESS assignment
	mac_pib.ChildRFDs = 0;
	mac_pib.ChildRouters = 0;
	mac_pib.nextChildRFD = macGetShortAddr() + 1+ ntGetCskip(mac_pib.depth+1)*(LRWPAN_MAX_ROUTERS_PER_PARENT);
	mac_pib.nextChildRouter = macGetShortAddr() + 1;
}



//adds a neighbor, and assigns a new SADDR
SADDR ntAddNeighbor(BYTE *ptr, BYTE capinfo) {
	NAYBORENTRY *nt_ptr;
	BYTE j;
	BYTE *tmpptr;

	//First, find free entry in neighbor table
    nt_ptr = &mac_nbr_tbl[0];
	for (j=0;j<NTENTRIES;j++,nt_ptr++) {
		if (!nt_ptr->flags.bits.used) break;
	}
	if (j== NTENTRIES) return(LRWPAN_BCAST_SADDR);//error, no room

	//now find free entry in address map table
	for (j=0;j<LRWPAN_MAX_ADDRESS_MAP_ENTRIES;j++) {
		if (mac_addr_tbl[j].saddr == LRWPAN_BCAST_SADDR) break;
	}
	if (j== LRWPAN_MAX_ADDRESS_MAP_ENTRIES) return(LRWPAN_BCAST_SADDR);//error, no room
    nt_ptr->map_index = j;
	nt_ptr->flags.bits.used = 1;
	nt_ptr->flags.bits.lqi = 0;
	nt_ptr->capinfo = capinfo;

	//now new saddr
	if (LRWPAN_GET_CAPINFO_DEVTYPE(capinfo)) {
		mac_addr_tbl[nt_ptr->map_index].saddr = mac_pib.nextChildRouter;
		mac_pib.nextChildRouter += ntGetCskip(mac_pib.depth+1);
		mac_pib.ChildRouters++;
	}else {
		mac_addr_tbl[nt_ptr->map_index].saddr = mac_pib.nextChildRFD;
        mac_pib.nextChildRFD++;
		mac_pib.ChildRFDs++;
	}
	//now copy long addr
	tmpptr = &mac_addr_tbl[nt_ptr->map_index].laddr[0];
	for(j=0;j<8;j++) {
		*tmpptr = *ptr;
		tmpptr++; ptr++;
	}
	return(mac_addr_tbl[nt_ptr->map_index].saddr);
}




//determine where this packet is going based on its short address
SADDR ntFindNewDst(SADDR dstSADDR){
	SADDR tmpSADDR;
	NAYBORENTRY *nt_ptr;
	BYTE j;

  if (dstSADDR == macGetShortAddr()) {
	  //trying to send to myself, this is an error
	  return(0xFFFF);
	}
  //if destination is coordinator, has to go to our parent
  if (dstSADDR == 0) return(mac_pib.macCoordShortAddress);
  // See if this destination is within my routing range
  // if not, then have to send it to my parent
#ifndef LRWPAN_COORDINATOR
  //do not check this for coordinator, as all nodes in range of coordinator.
  tmpSADDR = ntGetMaxSADDR(macGetShortAddr(),mac_pib.depth+1);
  if (!((dstSADDR > macGetShortAddr()) &&
	  (dstSADDR <= tmpSADDR))) {
		  //not in my range, must go to parent.
		  return(mac_pib.macCoordShortAddress);
	  }
#endif

  //goes to one of my children, check out each one.	  	
  nt_ptr = &mac_nbr_tbl[0];

  for (j=0;j<NTENTRIES;j++,nt_ptr++) {
		if (!nt_ptr->flags.bits.used) continue;
		if (LRWPAN_GET_CAPINFO_DEVTYPE(nt_ptr->capinfo)) {
			//router. check its range, the range is mac_pib.depth+2 because we need
			//the depth of the my child's child (grandchild).
			tmpSADDR = ntGetMaxSADDR(mac_addr_tbl[nt_ptr->map_index].saddr,mac_pib.depth+2);
			if ((dstSADDR >= mac_addr_tbl[nt_ptr->map_index].saddr) && (dstSADDR <= tmpSADDR)) {
				//either for my child router or one of its children.
				return(mac_addr_tbl[nt_ptr->map_index].saddr);
			}
		}else {
			//if for a non-router child, return
			if (dstSADDR == mac_addr_tbl[nt_ptr->map_index].saddr) return(mac_addr_tbl[nt_ptr->map_index].saddr);
		}
  }
  //if get here, then packet is undeliverable
  return(0xFFFF);
}


#endif
