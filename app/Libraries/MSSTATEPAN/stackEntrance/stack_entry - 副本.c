/*
 V0.1 Initial Release   10/July/2006  RBR

 */

/*
 This is a two node test, requires a Coordinator
 and an RFD. The coordinator and node simply
 ping-pong a packet back and forth, and print
 out the RSSI byte.  The RFD waits before
 bouncing it back, while the coordinator responds
 immediately.

 Expects coordinator, and one RFD.
 The topology to test should be:

 Coordinator ->  RFD1


 Start the coordinator first, then
 RFD1. If a RFD1 fails to join the network, try
 again. The RFD1 will prompt the user to hit
 a key to start the ping-pong.

 You can connect multiple RFDs if desired.

 You can also ping-pong through a router; see
 the note in usrJoinVerifyCallback(). The topology
 for a router would be:

 coord -> router -> RFD1
 -> RFD2
 -> ..RFDn


 This  requires Virtual Boards to be running,
 since a switch press is needed to start the pinging.


 */

#include "msstate_lrwpan.h"
#include "xprintf.h"

#ifndef LRWPAN_COORDINATOR
#define PING_DELAY   2  //wait before bouncing back
#else
#define PING_DELAY   0 //coordinator does not wait
#endif

#define RX_PING_TIMEOUT     5    //seconds
//this is assumed to be the long address of our coordinator, in little endian order
//used to test LONG ADDRESSING back to coordinator

UINT16 ping_cnt;
UINT32 my_timer;
UINT32 last_tx_start;

LADDR_UNION dstADDR;

typedef enum _PP_STATE_ENUM {
	PP_STATE_START_RX, PP_STATE_WAIT_FOR_RX, PP_STATE_SEND, PP_STATE_WAIT_FOR_TX
} PP_STATE_ENUM;

PP_STATE_ENUM ppState;
BYTE rxFlag; //set from within usrRxPacketCallback
BYTE payload[2];
UINT16 numTimeouts;
BOOL first_packet;

int count = 0;



void MyTest() {
#ifdef LRWPAN_COORDINATOR
	apsFSM(); //协调器只接收数据，不发送
#if 1
	if(rxFlag) {
		count=0;
		rxFlag = 0;

		//增加后重新发出
		ping_cnt++;//this was value received by this node

		payload[0] = (BYTE) ping_cnt;
		payload[1] = (BYTE) (ping_cnt >> 8);

		aplSendMSG (APS_DSTMODE_SHORT,
				&dstADDR,
				0x70,//dst EP
				0,//cluster is ignored for direct message
				0x71,//src EP
				&payload[0],
				2,//msg length
				apsGenTSN(),
				FALSE);//No APS ack requested

		printf("COORDINATOR Aps Send data=%d\n",ping_cnt);
	}

#endif

#else

	BYTE id_coord = 0x71; //协调器ID
	#if RFD_0
	BYTE id_rfd = 0x70;
	BYTE dst_ep=0x69;
	#elif RFD_1
	BYTE id_rfd=0x69;
	BYTE dst_ep=0x70;
	#elif RFD_2
	BYTE id_rfd=0x68;
	#else
	BYTE id_rfd=0;
	#endif


	apsFSM(); //wait for finish
	if (count++ % 5 == 0) {
		first_packet = FALSE; //触发
		rxFlag = 0;

		//增加后重新发出
		ping_cnt++; //this was value received by this node

		payload[0] = (BYTE) ping_cnt;
		payload[1] = (BYTE) (ping_cnt >> 8);

		aplSendMSG(APS_DSTMODE_SHORT, &dstADDR, id_coord, //dst EP
				0,//cluster is ignored for direct message
				id_rfd,//src EP
				&payload[0], 2,//msg length
				apsGenTSN(), FALSE);
		//No APS ack requested
		printf("RFD hwID=%x netID=%d Aps Send data=%d\n",id_rfd,aplGetMyShortAddress(), ping_cnt);
	}
	//有数据发送才会触发apsBusy，每次发送后对网络进行检测保证发送结束
	while (apsBusy()) {
		apsFSM(); //wait for finish
	}
	delayMS(500);	//
#endif

}

void stack_main(void) {

	//this initialization set our SADDR to 0xFFFF,
	//PANID to the default PANID

	//HalInit, evbInit will have to be called by the user

	numTimeouts = 0;
	my_timer = 0;
	first_packet = TRUE;
	//halInit();



	aplInit(); //init the stack
	conPrintConfig();

	EVB_LED1_OFF();
	EVB_LED2_OFF();

	ping_cnt = 0;
	rxFlag = 0;
	debug_level = 0;

#ifdef LRWPAN_COORDINATOR

	aplFormNetwork();
	while(apsBusy()) {
		apsFSM(); //wait for finish
	}
	conPrintROMString("Network formed, waiting for RX\n");
	EVB_LED1_ON();
	ppState = PP_STATE_START_RX;
#else
	do {
		aplJoinNetwork();
		while (apsBusy()) {
			apsFSM(); //wait for finish
		}
		if (aplGetStatus() == LRWPAN_STATUS_SUCCESS) {
			EVB_LED1_ON();
			conPrintROMString("Network Join succeeded!\n");
			conPrintROMString("My ShortAddress is: ");
			conPrintUINT16(aplGetMyShortAddress());
			conPCRLF();
			conPrintROMString("Parent LADDR: ")
			conPrintLADDR(aplGetParentLongAddress());
			conPrintROMString(", Parent SADDR: ");
			conPrintUINT16(aplGetParentShortAddress());
			conPCRLF();
			break;
		} else {
			conPrintROMString(
					"Network Join FAILED! Waiting 2s, then trying again\n");
			delayMS(2000);
		}
	} while (1);

#endif

#ifdef LRWPAN_RFD
	//now send packets
	dstADDR.saddr = 0; //RFD sends to the coordinator
	ppState = PP_STATE_SEND;

#endif

#if (defined(LRWPAN_RFD) || defined(LRWPAN_COORDINATOR))

	while (1) {
		MyTest();
	}
#endif

#ifdef LRWPAN_ROUTER
	//router does nothing, just routes
	DEBUG_PRINTNEIGHBORS(DBG_INFO);
	conPrintROMString("Router, doing its thing.!\n");
	while(1) {
		apsFSM();
	}
#endif

}

//########## Callbacks ##########

//callback for anytime the Zero Endpoint RX handles a command
//user can use the APS functions to access the arguments
//and take additional action is desired.
//the callback occurs after the ZEP has already taken
//its action.
LRWPAN_STATUS_ENUM usrZepRxCallback(void) {

#ifdef LRWPAN_COORDINATOR
	if (aplGetRxCluster() == ZEP_END_DEVICE_ANNOUNCE) {
		//a new end device has announced itself, print out the
		//the neightbor table and address map
		//dbgPrintNeighborTable();
	}
#endif
	return LRWPAN_STATUS_SUCCESS;
}

//callback from APS when packet is received
//user must do something with data as it is freed
//within the stack upon return.

LRWPAN_STATUS_ENUM usrRxPacketCallback(void) {

	BYTE len, *ptr, rssi;
	static UINT16 recv;
	//just print out this data

	len = aplGetRxMsgLen();
	rssi = aplGetRxRSSI();

	ptr = aplGetRxMsgData();
	recv = *ptr;
	ptr++;
	recv += ((UINT16) *ptr) << 8;

	rxFlag = 1; //signal that we got a packet
	//use this source address as the next destination address
	dstADDR.saddr = aplGetRxSrcSADDR();
#ifdef LRWPAN_COORDINATOR
	printf("COORDINATOR Get from ID=%d,data=%d,rssi=%d\n",dstADDR.saddr,recv,rssi);
#else
	printf("RFD Get from ID=%d,data=%d,rssi=%d\n", dstADDR.saddr, recv, rssi);
#endif

	return LRWPAN_STATUS_SUCCESS;
}

#ifdef LRWPAN_FFD
//Callback to user level to see if OK for this node
//to join - implement Access Control Lists here based
//upon IEEE address if desired
BOOL usrJoinVerifyCallback(LADDR *ptr, BYTE capinfo)
{

#if 0      //set this to '1' if you want to test through a router
//only accept routers.
//only let routers join us if we are coord
#ifdef LRWPAN_COORDINATOR
	if (LRWPAN_GET_CAPINFO_DEVTYPE(capinfo)) {
		//this is a router, let it join
		conPrintROMString("Accepting router\n");
		return TRUE;
	} else {
		conPrintROMString("Rejecting non-router\n");
		return FALSE;
	}
#else
	return TRUE;
#endif

#else

	return TRUE;

#endif

}

BOOL usrJoinNotifyCallback(LADDR *ptr)
{

	//allow anybody to join

	conPrintROMString("Node joined: ");
	conPrintLADDR(ptr);
	conPCRLF();
	DEBUG_PRINTNEIGHBORS(DBG_INFO);
	return TRUE;
}
#endif

//called when the slow timer interrupt occurs
#ifdef LRWPAN_ENABLE_SLOW_TIMER
void usrSlowTimerInt(void) {
}
#endif

//general interrupt callback , when this is called depends on the HAL layer.
void usrIntCallback(void) {
}
