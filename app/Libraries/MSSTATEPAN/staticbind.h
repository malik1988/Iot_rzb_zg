/*
 * 网络设备绑定，只限间接寻址使用， 直接寻址方式，不需要。
 *
 */


#ifndef STATICBIND_H
#define STATICBIND_H

#ifdef LRWPAN_COORDINATOR
#ifdef LRWPAN_USE_DEMO_STATIC_BIND

typedef struct  _BINDENTRY {
	LADDR src;
	BYTE  srcEP;
	LADDR dst;
	BYTE  dstEP;
	BYTE  cluster;
}BINDENTRY;

//modify this table to fill in your static binds!!!!
//for simplicity, this uses the LSB of the example nodes
//as the endpoint number

/* binds

First binding pair allows RFD1, RFD2 to exchange indirect messages
RFD1 (LSB= 0x70) -> RFD2 (LSB= 0x71)
RFD2 (LSB= 0x71) -> RFD1 (LSB= 0x70)


Second binding pair allows endpoint on Coordinator to exchange
indirect messages with RFD1

Coord(LSB=0x6F) -> RFD1 (LSB= 0x70)
RFD1 (LSB= 0x70) -> Coord(LSB=0x6F)


*/


BINDENTRY bindtable[] =

{
#if 0	//只有间接寻址才使用
	//this entry allows node 0x70 to send to 0x71
	{0x70,0x21,0x01,0x00,0x00,0x4B,0x12,0x00,  //SRC, little endian
	0x70,                                      //src endpoint
    0x71,0x21,0x01,0x00,0x00,0x4B,0x12,0x00,  //DST, little endian
	0x71,                                      //dst endpoint
	LRWPAN_APP_CLUSTER},                         //Cluster
	//this is the reverse direction, allows node 0x71 -> 0x70
    {0x71,0x21,0x01,0x00,0x00,0x4B,0x12,0x00,  //SRC, little endian
	0x71,                                      //src endpoint
    0x70,0x21,0x01,0x00,0x00,0x4B,0x12,0x00,  //DST, little endian
	0x70,                                      //dst endpoint
	LRWPAN_APP_CLUSTER},                         //Cluster
#endif

};


#endif
#endif

#endif




