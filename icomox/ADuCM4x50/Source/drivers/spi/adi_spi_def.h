/*! ****************************************************************************
 * @file:    adi_spi_def.h
 * @brief:   SPI Device Driver definition 
 * -----------------------------------------------------------------------------
 * Copyright (c) 2010-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_SPI_DEF_H_
#define ADI_SPI_DEF_H_



#include <drivers/spi/adi_spi.h>

#define ADI_SPI_NUM_INSTANCES         (3u)
#define ADI_SPI_FIFO_SIZE             (8u)
#define ADI_SPI_IRQ_PARAM             (7u)

/* Macro mapping from ADuCM4x50 to ADuCM302x */
#if defined(__ADUCM302x__)
#define DMA_SPIH_TX_Int_Handler(void) DMA_SPI2_TX_Int_Handler(void)
#define DMA_SPIH_RX_Int_Handler(void) DMA_SPI2_RX_Int_Handler(void)
#endif

 /*! \cond PRIVATE */

/*
 *****************************************************************************
 * SPI Bitrate Initializer.  Sets a default serial clockrate for the SPI channel.
 *****************************************************************************/
/* #define ADI_SPI_BITRATE_INITIALIZER   4000000 // 4MHz default bitrate */
#define ADI_SPI_BITRATE_INITIALIZER      250000u  /* depends on processor */

/*
 *****************************************************************************
 * SPI0/SPI1 Control Register Initializer.  This macro configures default
 * settings for the SPI configuration control register when operated in Master-mode.
 *****************************************************************************/
/* SPI master DMA mode control configuration */
#define ADI_SPI_MASTERCON_INITIALIZER      BITM_SPI_CTL_MASEN

/*
 *****************************************************************************
 * SPI0/SPI1 Control Register Initializer.  This macro configures default
 * settings for the SPI configuration control register when operated in Slave-mode.
 *****************************************************************************/
 #define ADI_SPI_SLAVECON_INITIALIZER  BITM_SPI_CTL_OEN           \
                                     | BITM_SPI_CTL_ZEN           \
                                     | BITM_SPI_CTL_SPIEN

/* 16-bit DMA... (two-byte size and increment) */
#define ADI_DMA_DATA_WIDTH      ADI_DMA_WIDTH_2_BYTE  /*!< DMA data attribute */
#define ADI_DMA_DATA_INCREMENT  ADI_DMA_INCR_HALFWORD   /*!< DMA data attribute */



/*!
 *****************************************************************************
 * SPI Configuration structure.
 *****************************************************************************/
typedef struct ADI_SPI_CONFIG
{
    uint16_t SPI_CTL;                          /*!< SPI_CTL register configuration. */
    uint16_t SPI_DIV;                          /*!< SPI_DIV  register.     */
} ADI_SPI_CFG_TYPE;

/*! SPI device information */

typedef struct __ADI_SPI_DEVICE_INFO
{
    const uint16_t               dmaTxIrqNumber;     /*  DMA channel ID-Tx */
    const uint16_t               dmaTxChannelNumber; /*  Tx */
    const uint16_t               dmaRxIrqNumber;     /*  DMA channel ID-Rx */    
    const uint16_t               dmaRxChannelNumber; /*  DMA channel ID-Rx */    
    volatile ADI_SPI_TypeDef     *pSpiRegs;          /*  Base address of the  SPI registers */
    const IRQn_Type              eIRQn;              /*  IRQn */   
    ADI_SPI_HANDLE               hDevice;            /*  SPI handle */
}ADI_SPI_DEVICE_INFO;


/*! \struct ADI_SPI_DEV_DATA_TYPE SPI Device instance data structure */
typedef struct __ADI_SPI_DEV_DATA_TYPE
{

    /* device attributes */
    volatile ADI_SPI_TypeDef *pSpi;              /*!< track MMR device pointer   */
    ADI_SPI_DEVICE_INFO      *pDevInfo;

    /* Callback and Callback parameters */
    ADI_CALLBACK             pfCallback;         /*!< Callback address          */
    void *                   pCBParam;           /*!< Callback parameter        */
    /* The last recorded SPI event */
    uint32_t                 HWErrors;           /*!< HW transaction status */

    uint8_t*                 pTxBuffer;          /*!< Transmit Buffer           */
    uint8_t*                 pRxBuffer;          /*!< Receive Buffer            */
    uint16_t                 TxRemaining;        /*!< Transmit Count            */
    uint16_t                 RxRemaining;        /*!< Receive Count             */
    uint8_t                  TxIncrement;        /*!< Transmit Increment        */
    uint8_t                  RxIncrement;        /*!< Receive Increment         */
    
    volatile bool            bTransferComplete;  /*!< Transfer Complete Flag    */

    bool                     bDmaMode;           /*!< DMA mode flag              */
    bool                     bRdCtlMode;         /* Use half duplex read control feature */
    bool                     bBlockingMode;      /*!< blocking mode flag         */
    ADI_SPI_CHIP_SELECT      ChipSelect;         /*!< track chip select          */
    
    SEM_VAR_DECLR
} ADI_SPI_DEV_DATA_TYPE;



/*! \endcond */

#endif	/* ADI_SPI_DEF_H__ */

