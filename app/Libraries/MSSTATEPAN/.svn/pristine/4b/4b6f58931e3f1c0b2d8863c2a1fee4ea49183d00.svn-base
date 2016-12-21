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


#ifndef NEIGHBOR_H
#define NEIGHBOR_H



typedef struct _MAPENTRY {
	UINT16 saddr;   //an address of 0xFFFF means this is unused
	//an address of 0xFFFE means this is unknown or not assigned
	BYTE laddr[8];	                
}MAPENTRY;
//holds pairs of long/short addresses, first entry is always our own.
extern MAPENTRY mac_addr_tbl[LRWPAN_MAX_ADDRESS_MAP_ENTRIES ];
SADDR ntNewAddressMapEntry(BYTE *laddr, SADDR saddr);
BOOL ntFindAddressBySADDR(SADDR saddr, BYTE *index);
BOOL ntFindAddressByLADDR(LADDR *ptr, BYTE *index);
void ntAddOurselvesToAddressTable(SADDR saddr);
void ntInitAddressMap(void);
SADDR ntGetMaxSADDR(SADDR router_saddr,BYTE depth);
UINT16 ntGetCskip(BYTE depth);

//rest of this only needed for FFDs
#ifdef LRWPAN_FFD
//only needed for FFDs

#define NTENTRIES (LRWPAN_MAX_CHILDREN_PER_PARENT)

//Howdy NAYBOR!!!!  I dislike spelling.
typedef struct _NAYBORENTRY {

	BYTE    map_index;   //index of corresponding laddr/saddr pair in address table
	BYTE    capinfo;     //node capability info
	union {
		BYTE val;
		struct {
			unsigned used: 1;  //true if used
			unsigned lqi: 7;   //link quality indictor
		}bits;
	}flags;
}NAYBORENTRY;


//holds the neighbor table entries.
extern NAYBORENTRY mac_nbr_tbl[NTENTRIES];


NAYBORENTRY *ntFindBySADDR (UINT16 saddr);
NAYBORENTRY *ntFindByLADDR (LADDR *ptr);
void ntInitTable(void);
SADDR ntAddNeighbor(BYTE *ptr, BYTE capinfo);
void ntInitAddressAssignment(void);
SADDR ntFindNewDst(SADDR dstSADDR);


#endif

#endif


