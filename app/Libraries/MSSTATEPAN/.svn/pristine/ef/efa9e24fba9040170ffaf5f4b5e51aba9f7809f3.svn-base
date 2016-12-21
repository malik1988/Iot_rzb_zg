/*!
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* All rights reserved.
*
* \file MCR20Drv.h
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


/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */

#include "EmbeddedTypes.h"
#include "SPI.h"
#include "fsl_clock_manager.h"
#include "pin_mux.h"

#if FSL_FEATURE_SOC_DSPI_COUNT
    #include "fsl_dspi_hal.h"
    #include "fsl_dspi_master_driver.h"
#else
    #include "fsl_spi_hal.h"
    #include "fsl_spi_master_driver.h"
#endif


/*! *********************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
********************************************************************************** */
#if FSL_FEATURE_SOC_DSPI_COUNT
uint32_t mDspiCmd;
const dspi_command_config_t mSpiCommand = {
    .isChipSelectContinuous = FALSE,
    .whichCtar = kDspiCtar0,
    .whichPcs = kDspiPcs0,
    .isEndOfQueue = TRUE,
    .clearTransferCount = TRUE
};

const dspi_data_format_config_t mDspiCfg = {
    .bitsPerFrame = 8,
    .clkPolarity = kDspiClockPolarity_ActiveHigh,
    .clkPhase = kDspiClockPhase_FirstEdge,
    .direction = kDspiMsbFirst
};
#else
uint8_t mSpiLowSpeed, mSpiHighSpeed;
#endif


/*! *********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
********************************************************************************** */
void spi_master_init(uint32_t instance)
{
    SPI_Type *baseAddr;

    /* set SPI Pin Mux */    
    configure_spi_pins(instance);
    /* Enable SPI clock */
    CLOCK_SYS_EnableSpiClock(instance);

#if FSL_FEATURE_SOC_DSPI_COUNT
    baseAddr = g_dspiBase[instance];

    /* Initialize SPI module */
    DSPI_HAL_Init(baseAddr);
    DSPI_HAL_SetMasterSlaveMode(baseAddr, kDspiMaster);
    DSPI_HAL_SetContinuousSckCmd(baseAddr, FALSE);
    DSPI_HAL_SetPcsPolarityMode(baseAddr, kDspiPcs0, kDspiPcs_ActiveLow);
    DSPI_HAL_SetFifoCmd(baseAddr, TRUE, TRUE);

    DSPI_HAL_SetDataFormat(baseAddr, kDspiCtar0, &mDspiCfg);
    DSPI_HAL_SetDataFormat(baseAddr, kDspiCtar1, &mDspiCfg);

    DSPI_HAL_SetBaudRate(baseAddr, kDspiCtar0,  8000000, CLOCK_SYS_GetSpiFreq(instance));
    DSPI_HAL_SetBaudRate(baseAddr, kDspiCtar1, 16000000, CLOCK_SYS_GetSpiFreq(instance));

    mDspiCmd = DSPI_HAL_GetFormattedCommand(baseAddr, (dspi_command_config_t*)&mSpiCommand );

    DSPI_HAL_Enable(baseAddr);
    DSPI_HAL_StartTransfer(baseAddr);
#else
    baseAddr = g_spiBase[instance];

    SPI_HAL_Init(baseAddr);
    SPI_HAL_SetMasterSlave(baseAddr, kSpiMaster);
    SPI_HAL_Enable(baseAddr);

    SPI_HAL_SetBaud(baseAddr, 8000000, CLOCK_SYS_GetSpiFreq(instance));
    mSpiLowSpeed = SPI_RD_BR(baseAddr);

    SPI_HAL_SetBaud(baseAddr, 16000000, CLOCK_SYS_GetSpiFreq(instance));
    mSpiHighSpeed = SPI_RD_BR(baseAddr);
#endif
}

/*****************************************************************************/
/*****************************************************************************/
void spi_master_configure_speed(uint32_t instance, uint32_t freq)
{
    if( freq > 8000000 )
    {
#if FSL_FEATURE_SOC_DSPI_COUNT
        mDspiCmd |= 1 << SPI_PUSHR_CTAS_SHIFT;
#else
        SPI_WR_BR(g_spiBase[instance], mSpiHighSpeed);
#endif
    }
    else
    {
#if FSL_FEATURE_SOC_DSPI_COUNT
        mDspiCmd &= ~SPI_PUSHR_CTAS_MASK;
#else
        SPI_WR_BR(g_spiBase[instance], mSpiLowSpeed);
#endif
    }
}

/*****************************************************************************/
/*****************************************************************************/
void spi_master_transfer(uint32_t instance,
                         uint8_t * sendBuffer,
                         uint8_t * receiveBuffer,
                         size_t transferByteCount)
{
    volatile uint8_t dummy;
#if FSL_FEATURE_SOC_DSPI_COUNT
    SPI_Type *baseAddr = g_dspiBase[instance];
#else
    SPI_Type *baseAddr = g_spiBase[instance];
#endif

    if( !transferByteCount )
        return;

    if( !sendBuffer && !receiveBuffer )
        return;

#if FSL_FEATURE_SOC_DSPI_COUNT
    DSPI_HAL_SetFlushFifoCmd(baseAddr, true, true);
#endif

    while( transferByteCount-- )
    {
        if( sendBuffer )
        {
#if FSL_FEATURE_SOC_DSPI_COUNT
            ((uint8_t*)&mDspiCmd)[0] = *sendBuffer;
#else
            dummy = *sendBuffer;
#endif
            sendBuffer++;
        }

#if FSL_FEATURE_SOC_DSPI_COUNT
        DSPI_HAL_WriteCmdDataMastermodeBlocking(baseAddr, mDspiCmd);
        dummy = DSPI_HAL_ReadData(baseAddr);
#else
  #if FSL_FEATURE_SPI_16BIT_TRANSFERS
        SPI_HAL_WriteDataBlocking(baseAddr, kSpi8BitMode, 0, dummy);
        while(!SPI_HAL_IsReadBuffFullPending(baseAddr));
        dummy = SPI_HAL_ReadDataLow(baseAddr);
  #else
        SPI_HAL_WriteDataBlocking(baseAddr, dummy);
        dummy = SPI_HAL_ReadData(baseAddr);
  #endif
#endif

        if( receiveBuffer )
        {
            *receiveBuffer = dummy;
            receiveBuffer++;
        }
    }
}

/*****************************************************************************/
/*****************************************************************************/
inline void spi_master_configure_serialization_lsb(uint32_t instance)
{
#if FSL_FEATURE_SOC_DSPI_COUNT
    SPI_Type * baseAddr = g_dspiBase[instance];

    SPI_BWR_CTAR_LSBFE(baseAddr, kDspiCtar0, kDspiLsbFirst);
    SPI_BWR_CTAR_LSBFE(baseAddr, kDspiCtar1, kDspiLsbFirst);
#else
    SPI_WR_C1_LSBFE(g_spiBase[instance], kSpiLsbFirst);
#endif
}

/*****************************************************************************/
/*****************************************************************************/
inline void spi_master_configure_serialization_msb(uint32_t instance)
{
#if FSL_FEATURE_SOC_DSPI_COUNT
    SPI_Type * baseAddr = g_dspiBase[instance];

    SPI_BWR_CTAR_LSBFE(baseAddr, kDspiCtar0, kDspiMsbFirst);
    SPI_BWR_CTAR_LSBFE(baseAddr, kDspiCtar1, kDspiMsbFirst);
#else
    SPI_WR_C1_LSBFE(g_spiBase[instance], kSpiMsbFirst);
#endif
}