/*!
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* All rights reserved.
*
* \file
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of Freescale Semiconductor, Inc. nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _PHY_TYPES_H
#define _PHY_TYPES_H


/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
********************************************************************************** */

/* WARNING!!! Only one frequency can be declared at a time! */
#ifdef gPHY_802_15_4g_d

  /* 802.15.4g Frequency Bands */
  #ifndef gFreqBand_169_400__169_475MHz_d
  #define gFreqBand_169_400__169_475MHz_d 0
  #endif

  #ifndef gFreqBand_470__510MHz_d
  #define gFreqBand_470__510MHz_d 0
  #endif

  #ifndef gFreqBand_779__787MHz_d
  #define gFreqBand_779__787MHz_d 0
  #endif

  #ifndef gFreqBand_863__870MHz_d
  #define gFreqBand_863__870MHz_d 0
  #endif

  #ifndef gFreqBand_902__928MHz_d
  #define gFreqBand_902__928MHz_d 1
  #endif

  #ifndef gFreqBand_920__928MHz_d
  #define gFreqBand_920__928MHz_d 0       
  #endif

  /* Non-standard Frequency Bands */
  #ifndef gFreqBand_865__867MHz_d
  #define gFreqBand_865__867MHz_d 0
  #endif

  #if gFreqBand_169_400__169_475MHz_d
    #define gFreqBandId_d           gFreq169_400__169_475MHz_c  
  #elif gFreqBand_470__510MHz_d
    #define gFreqBandId_d           gFreq470__510MHz_c
  #elif gFreqBand_779__787MHz_d
    #define gFreqBandId_d           gFreq779__787MHz_c
  #elif gFreqBand_863__870MHz_d
    #define gFreqBandId_d           gFreq863__870MHz_c
  #elif gFreqBand_902__928MHz_d
    #define gFreqBandId_d           gFreq902__928MHz_c
  #elif gFreqBand_920__928MHz_d
    #define gFreqBandId_d           gFreq920__928MHz_c
  #elif gFreqBand_865__867MHz_d
    #define gFreqBandId_d           gFreq865__867MHz_c
  #else
    #error "No frequency band declared!"
  #endif
#endif  /* gPHY_802_15_4g_d */

/// \note MUST REMAIN UNCHANGED:
#ifdef gPHY_802_15_4g_d
  #define gPhySymbolsPerOctet_c     8    
  #define gPhyMRFSKPHRLength_c      2    /* [bytes] */
  #define gPhyFSKPreambleLength_c   16   /* [bytes] */
  #define gPhyMRFSKSFDLength_c      2    /* [bytes] */
  #define gMinPHYPacketSize_c       5
  #define gMaxPHYPacketSize_c       254  /* maximum number of bytes that the PHY can transmit or receive */
  #define gPhyFCSSize_c             2    /* [bytes] */  
  #define gCCADurationDefault_c     13   /* [symbols] */
  #define gPhySHRDuration_c        (gPhySymbolsPerOctet_c * (gPhyFSKPreambleLength_c + gPhyMRFSKSFDLength_c)) /* [symbols] */
  #define gPhyMaxFrameDuration_c   (gPhySHRDuration_c + (gPhyMRFSKPHRLength_c + gMaxPHYPacketSize_c) * gPhySymbolsPerOctet_c)  /* [symbols] 802.15.4g page 48 formula  */
#else
  #define gCCATime_c                8   /* [symbols] */
  #define gPhyTurnaroundTime_c      12  /* [symbols] RX-to-TX or TX-to-RX maximum turnaround time (in symbol periods)*/
  #define gMinPHYPacketSize_c       5
  #define gMaxPHYPacketSize_c       (127)  /* maximum number of bytes that the PHY can transmit or receive */
  #define gPhySHRDuration_c         (10)   /* [symbols] */
  #define gPhySymbolsPerOctet_c     (2)
  #define gPhyFCSSize_c             (2)    /* [bytes] */
  #define gPhyMaxFrameDuration_c    (gPhySHRDuration_c + (gMaxPHYPacketSize_c + 1) * gPhySymbolsPerOctet_c)
  #define gUnitBackoffPeriod_c      (20)  /* [symbols] */
#endif  /* gPHY_802_15_4g_d */

/* Phy flags */
#define gPhyFlagRxOnWhenIdle_c  (1 << 0)
#define gPhyFlagIdleRx_c        (1 << 1)
#define gPhyFlagRxFP_c          (1 << 2)
#define gPhyFlagTxAckFP_c       (1 << 3)
#define gPhyFlagDeferTx_c       (1 << 4)


#ifdef gPHY_802_15_4g_d
#ifndef gAfcRxTimeout_c
#define gAfcRxTimeout_c             (gPhySHRDuration_c) /* [symbols] - Min Preamble + SFD + variable duration */
#endif

#ifndef gRssiThreshold_c
#define gRssiThreshold_c        0xB4 /* -90 dBm */
#endif

#ifndef gPhyModeDefault_d
#define gPhyModeDefault_d gPhyMode1_c
#endif
#endif /* #ifdef gPHY_802_15_4g_d */

#define gInvalidTimerId_c (0xFF)

#ifdef gPHY_802_15_4g_d
#define gPhyTimeShift_c   (16)          /* 16bit hw timer */
#define gPhyTimeMask_c    (0x000000000000FFFF)
#else
#define gPhyTimeShift_c   (24)          /* 24bit hw timer */
#define gPhyTimeMask_c    (0x00FFFFFF)
#endif

/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */
typedef enum
{
    gPhyChannelBusy_c          = 0x00,    /*The CCA attempt has detected a busy channel.*/
    gPhyBusyRx_c               = 0x01,    /*The transceiver is asked to change its state while receiving.*/
    gPhyBusyTx_c               = 0x02,    /*The transceiver is asked to change its state while transmitting.*/
    gPhyChannelIdle_c          = 0x04,    /*The CCA attempt has detected an idle channel.*/
    gPhyInvalidParameter_c     = 0x05,    /*A SET request was issued with a parameter in the primitive that is out of the valid range.*/
    gPhyRxOn_c                 = 0x06,    /*The transceiver is in the receiver enabled state.*/
    gPhySuccess_c              = 0x07,    /*A SET/GET, an ED operation, a data request, an indirect queue insert, or a transceiver state change was successful.*/
    gPhyTRxOff_c               = 0x08,    /*The transceiver is in the transceiver disabled state.*/
    gPhyTxOn_c                 = 0x09,    /*The transceiver is in the transmitter enabled state.*/
    gPhyUnsupportedAttribute_c = 0x0a,    /*A SET/GET request was issued with the identifier of an attribute that is not supported.*/
    gPhyReadOnly_c             = 0x0b,    /*A SET request was issued with the identifier of an attribute that is read-only.*/
    gPhyIndexUsed_c            = 0x11,    /*The indirect queue insert operation has detected an used index.*/
    gPhyNoAck_c                = 0x14,    /*No ACK was received for the last transmission.*/
    gPhyFramePending_c         = 0x15,	  /*The ACK of a Data Request frame indicates a pending frame in the coordinator’s indirect TX queue.*/
    gPhyBusy_c                 = 0xF1,    
    gPhyInvalidPrimitive_c     = 0xF2     
}phyStatus_t;

typedef enum
{
    gPhySlottedMode_c      = 0x0c,
    gPhyUnslottedMode_c    = 0x0d
}phySlottedMode_t;

typedef enum
{
    gPhyEnergyDetectMode_c = 0x00,
    gPhyCCAMode1_c         = 0x01,
    gPhyCCAMode2_c         = 0x02,
    gPhyCCAMode3_c         = 0x03,
    gPhyNoCCABeforeTx_c    = 0x04
}phyCCAType_t;

typedef enum
{
    gPhyContCcaEnabled     = 0x00,
    gPhyContCcaDisabled
}phyContCCAMode_t;

typedef enum
{
    gPhyForceTRxOff_c = 0x03,              /*The transceiver is to be switched off immediately.*/
    gPhySetRxOn_c     = 0x12,              /*The transceiver is to be configured into the receiver enabled state.*/
    gPhySetTRxOff_c   = 0x13,              /*The transceiver is to be configured into the transceiver disabled state.*/
}phyState_t;

typedef enum
{
    gPhyRxAckRqd_c     = 0x00,             /*A receive Ack frame is expected to follow the transmit frame (non-Ack frames are rejected)*/    
    gPhyNoAckRqd_c     = 0x01,             /*An ordinary receive frame (any type of frame) follows the transmit frame*/
#ifdef gPHY_802_15_4g_d     
    gPhyEnhancedAckReq = 0x02
#endif /* gPHY_802_15_4g_d */
}phyAckRequired_t;

typedef enum
{
    gPhyPibCurrentChannel_c  = 0x00,       /*The channel currently used.*/
    gPhyPibCurrentPage_c     = 0x01,       /*The channel page currently used.*/
    gPhyPibTransmitPower_c   = 0x02,       /*The power used for TX operations.*/
    gPhyPibLongAddress_c     = 0x03,       /*The MAC long address to be used by the PHY’s source address matching feature.*/
    gPhyPibShortAddress_c    = 0x04,       /*The MAC short address to be used by the PHY’s source address matching feature.*/
    gPhyPibPanId_c           = 0x05,       /*The MAC PAN ID to be used by the PHY’s source address matching feature.*/
    gPhyPibPanCoordinator_c  = 0x06,       /*Indicates if the device is a PAN coordinator or not.*/
    gPhyPibSrcAddrEnable_c   = 0x07,       /*Enables or disables the PHY’s source address matching feature.*/
    gPhyPibPromiscuousMode_c = 0x08,       /*Selects between normal, promiscuous and active promiscuous mode.*/
    gPhyPibAutoAckEnable_c   = 0x09,       /*Enables or disables automatic transmission of ACK frames.*/
    gPhyPibFrameVersion_c    = 0x0A,       /*Used in checking for allowed frame versions (0x00 - any version accepted, 0x01 - accept Frame Version 0 packets (2003 compliant), 0x02 - accept Frame Version 1 packets (2006 compliant), 0x03 - accept Frame Version 0 and 1 packets).*/
    gPhyPibFrameEnable_c     = 0x0B,       /*Used for enabling or disabling reception of MAC frames.*/
    gPhyPibAckFramePending_c = 0x0C,       /*Used to copy its contents to the outgoing ACK frame's Frame Pending field as a response to a received Data Request frame with Source Address Matching disabled.*/
    gPhyPibRxOnWhenIdle      = 0x0D,       /*Enable RX when the radio is IDLE*/  
    gPhyPibFrameWaitTime_c   = 0x0E,       /*The number of symbols the Rx should be on after receiving an ACK with FP=1 */
    gPhyPibDeferTxIfRxBusy_c = 0x0F,
    gPhyPibLastTxAckFP_c     = 0x10,
#ifdef gPHY_802_15_4g_d      
    gPhyPibCurrentMode_c            = 0x20,
    gPhyPibFSKPreambleRepetitions_c = 0x21,
    gPhyPibFSKScramblePSDU_c        = 0x22,
    gPhyPibCCADuration_c            = 0x23,
    gPhyPibCSLRxEnabled_c           = 0x24,
    gPhyPibFreqBandId_c             = 0x25,
    gPhyPibAckWaitDuration_c        = 0x26,
    gPhyPibTschEnabled_c            = 0x27,
    gPhyPibTschTimeslotInfo_c       = 0x28
#endif  /* gPHY_802_15_4g_d */
}phyPibId_t;

typedef struct phyFlags_tag
{
    union{
        uint32_t mask;
        struct{
            uint32_t     rxOnWhenIdle            :1;
            uint32_t     rxFramePending          :1;
            uint32_t     idleRx                  :1;
#ifdef  gPHY_802_15_4g_d
            uint32_t     ccaBfrTX                :1;
            uint32_t     rxAckRqd                :1;
            uint32_t     autoAck                 :1;
            uint32_t     panCordntr              :1;
            uint32_t     promiscuous             :1;
            uint32_t     activePromiscuous       :1;
            uint32_t     cslRxEnabled            :1;
            uint32_t     rxEnhAckRqd             :1;
            uint32_t     ccaComplete             :1;
            uint32_t     tschEnabled             :1;
            uint32_t     filterFail              :1;
            uint32_t     rxIsListen              :1;
            uint32_t     reserved                :17;            
#else
            uint32_t     phyState                :3;
            uint32_t     reserved                :26;
#endif  /* gPHY_802_15_4g_d */
        };
    };
}phyFlags_t;

typedef uint8_t  phyTimeTimerId_t;
typedef uint64_t phyTime_t;

typedef void (*phyTimeCallback_t) ( uint32_t param );

typedef enum
{
    gPhyTimeOk_c               = 0x00,
    gPhyTimeAlreadyPassed_c    = 0x01,
    gPhyTimeTooClose_c         = 0x02,
    gPhyTimeTooMany_c          = 0x03,
    gPhyTimeInvalidParameter_c = 0x04,
    gPhyTimeNotFound_c         = 0x05,
    gPhyTimeError_c            = 0x06
}phyTimeStatus_t;

#ifdef gPHY_802_15_4g_d
typedef struct phyPHR_tag
{
    union{
        uint16_t mask;
        uint8_t  byteAccess[2];
        struct{
            uint8_t     modeSwitch          :1;
            uint8_t     reserved            :2;
            uint8_t     fcsType             :1;
            uint8_t     dataWhitening       :1;
            uint8_t     frameLengthRsvd     :3; /* Max psdu 254 */
            uint8_t     frameLength;
        };
    };
}phyPHR_t;

/* 802.15.4g MAC sub-GHz Phy Modes */
typedef enum{
    gPhyMode1_c = 0x00, 
    gPhyMode2_c = 0x01, 
    gPhyMode3_c = 0x02,
    gPhyMode4_c = 0x03,
    /* Non-standard Phy Modes */
    gPhyMode1ARIB_c = 0x04,
    gPhyMode2ARIB_c = 0x05,
    gPhyMode3ARIB_c = 0x06, 
}phyMode_t;

/* 802.15.4g MAC sub-GHz frequency bands */
typedef enum{
    /* 802.15.4g Frequency Bands */
    gFreq169_400__169_475MHz_c = 0x00,    /* 169.400-169.475   (Europe) */
    gFreq470__510MHz_c = 0x02,    /* 470-510   (China)  */
    gFreq779__787MHz_c = 0x03,    /* 779-787   (China)  */
    gFreq863__870MHz_c = 0x04,    /* 863-870   (Europe) */
    gFreq902__928MHz_c = 0x07,    /* 902-928   (U.S.)   */
    gFreq920__928MHz_c = 0x09,    /* 920-928   (Japan) - Includes ARIB modes */
    /* Non-standard Frequency Bands */
    gFreq865__867MHz_c = 0x0E,    /* 865-867   (India)  */
}phyFreqBand_t;

typedef struct phyTschTsInfo_tag{
    uint16_t tsCCA;
    uint16_t tsRxTx;
    uint16_t tsRxAckDelay;
    uint16_t tsAckWait;
}phyTschTsInfo_t;
#endif  /* gPHY_802_15_4g_d */

/*! *********************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
********************************************************************************** */

#endif  /* _PHY_TYPES_H */
