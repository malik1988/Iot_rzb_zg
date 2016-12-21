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


/************************************************************************************
*@名称  StaticBind.c
*@概述  静态路由绑定
*@备注
************************************************************************************/


#include "compiler.h"               //compiler specific
#include "hal.h"
#include "halStack.h"
#include "lrwpan_config.h"
#include "evboard.h"
#include "neighbor.h"


#ifdef LRWPAN_COORDINATOR
#ifdef LRWPAN_USE_DEMO_STATIC_BIND
#include "staticbind.h"

//the prototypes for these functions are defined in evboard.h
//since binding other than these default static bind functions
//is assumed to be platform dependent.

static BYTE bind_table_index;
static BYTE src_map_index;
static BYTE bindSrcEP, bindCluster;


//this initializes the bindTable interator
BOOL evbBindTableIterInit(BYTE srcEP, SADDR srcSADDR, BYTE cluster)
{
    bind_table_index = 0;
    bindSrcEP = srcEP;
    bindCluster = cluster;
    //look up this srcSADDR in the address map table
    //if SADDR is not in the address table, this function returns FALSE, indicating failure
    return(ntFindAddressBySADDR(srcSADDR, &src_map_index));
}

//find the next binding that matches the current bindSrcEP, bindSrcSADDR, bindCluster
BOOL evbResolveBind(BYTE *dstEP, SADDR *dstSADDR)
{
    BYTE i, dst_map_index;
    BINDENTRY *bptr;

    //check if at end, if yes, exit.
    if (bind_table_index == sizeof(bindtable)) return(FALSE);



    do {
        bptr = &(bindtable[bind_table_index]);
        if ( (bptr->srcEP == bindSrcEP) &&
             (bptr->cluster == bindCluster)) {
            //match on EP and cluster
            //now check the SRC long address
            for (i = 0; i < 8; i++) {
                if (bptr->src.bytes[i] != mac_addr_tbl[src_map_index].laddr[i]) break;
            }
            if (i == 8) {
                if (ntFindAddressByLADDR(&bptr->dst, &dst_map_index)) {
                    //successfully found the dstSADDR
                    *dstEP = bptr->dstEP;
                    *dstSADDR = mac_addr_tbl[dst_map_index].saddr;
                    //before leaving, increment BPTR to point to next map entry
                    bind_table_index++;
                    return(TRUE);  //exit with match
                }

            }
        }
        //no match, continue looking
        bind_table_index++;
    } while(bind_table_index < sizeof(bindtable));

//if reach here, no match in entire table, so halt search
    return(FALSE);
}

#endif
#endif

