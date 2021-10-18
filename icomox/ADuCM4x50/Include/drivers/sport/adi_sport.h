/*! ****************************************************************************
 * @file    adi_sport.h
 * @brief   SPORT (Serial Port) Device driver definitions
 * @details Header File for the SPORT driver API functions and definitions
 -----------------------------------------------------------------------------
 * Copyright (c) 2016-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_SPORT_H
#define ADI_SPORT_H

/*=============  I N C L U D E S   =============*/

#include <adi_processor.h>
#include <rtos_map/adi_rtos_map.h>
#include <drivers/dma/adi_dma.h>
#include <adi_callback.h>

/** @addtogroup SPORT_Driver SPORT Driver
*  @{
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*==============  D E F I N E S  ===============*/

/**
 * Amount of memory (bytes) required by the SPORT device driver for managing
 * the operation in interrupt mode. This memory is completely owned by the
 * driver till the end of the operation.
 */
#define ADI_SPORT_MEMORY_SIZE   (76u + ADI_SEM_SIZE)

typedef void*  ADI_SPORT_HANDLE;        /*!< Handle to the SPORT Device */

/**
 * Enumeration of different channels of the SPORT
 */
typedef enum
{
    ADI_HALF_SPORT_A = 0,               /*!< First half SPORT */
    ADI_HALF_SPORT_B = 1                /*!< Second half SPORT */
} ADI_SPORT_CHANNEL;

/**
 * Enumeration for the direction of operation.
 */
typedef enum
{
    ADI_SPORT_DIR_RX,                   /*!< Sport in Rx mode */
    ADI_SPORT_DIR_TX                    /*!< Sport in Tx mode */
} ADI_SPORT_DIRECTION;

/**
 * Enumeration for enabling packing.
 */
typedef enum
{
    ADI_SPORT_NO_PACKING = 0,                                   /*!< No Packing */
    ADI_SPORT_8BIT_PACKING = ENUM_SPORT_CTL_A_CTL_PACK_8BIT,    /*!< 8-bit packing */
    ADI_SPORT_16BIT_PACKING = ENUM_SPORT_CTL_A_CTL_PACK_16BIT   /*!< 16-Bit packing */
} ADI_SPORT_PACKING_MODE;

/**
 * Enumeration for Hardware Error encountered by the SPORT device.
 */
 typedef enum
{
    ADI_SPORT_HW_NO_ERR                  = 0x00,  /*!< No Hardware error */
    ADI_SPORT_HW_ERR_RX_OVERFLOW         = 0x02,  /*!< Data overflow  for Rx (same value as Tx underflow) */
    ADI_SPORT_HW_ERR_TX_UNDERFLOW        = 0x02,  /*!< Data underflow for Tx (same value as Rx overflow) */
    ADI_SPORT_HW_ERR_FS                  = 0x04,  /*!< Frame sync error */
    ADI_SPORT_HW_ERR_SYSDATAERR          = 0x10,  /*!< System Data Error */

    ADI_SPORT_EVENT_RX_BUFFER_PROCESSED  = 0x20,  /*!< Processed the submitted RX buffer  */
    ADI_SPORT_EVENT_TX_BUFFER_PROCESSED  = 0x40,  /*!< Processed the submitted TX buffer  */

    ADI_SPORT_DMA_ERR_BUS                = 0x100, /*!< SPORT DMA bus error detected  */
    ADI_SPORT_DMA_ERR_INVALID_DESCRIPTOR = 0x200  /*!< SPORT DMA invalid descriptor error detected  */
}ADI_SPORT_EVENT;


/**
 *  Enumeration for result code returned from the SPORT device driver functions.
 */
typedef enum
{
    ADI_SPORT_SUCCESS,                  /*!< Success */
    ADI_SPORT_FAILED,                   /*!< Generic Failure to indicate a call to SPORT driver function returned unsuccessful */
    ADI_SPORT_INVALID_DEVICE_NUM ,      /*!< Invalid device number */
    ADI_SPORT_INVALID_NULL_POINTER,     /*!< Specified pointer is invalid */
    ADI_SPORT_INVALID_HANDLE,           /*!< The given handle is invalid */
    ADI_SPORT_INVALID_PARAMETER,        /*!< Specified parameter is not valid */
    ADI_SPORT_DMA_REGISTER_FAILED,      /*!< Registering DMA error handler failed */
    ADI_SPORT_DEVICE_IN_USE,            /*!< The specified SPORT channel is already open and in use */
    ADI_SPORT_INVALID_CONFIGURATION,    /*!< The SPORT configuration is invalid */
    ADI_SPORT_BUFFERS_NOT_SUBMITTED,    /*!< Buffer submission failed */
    ADI_SPORT_INVALID_WORD_LENGTH,      /*!< Invalid word size */
    ADI_SPORT_OPERATION_NOT_ALLOWED,    /*!< Specified operation is not allowed when SPORT is transmitting/receiving data */
    ADI_SPORT_HW_ERROR                  /*!< SPORT hardware or DMA reports an error  */
} ADI_SPORT_RESULT;

/*=============  P U B L I C   F U N C T I O N S =============*/

/* Opens a SPORT device */
ADI_SPORT_RESULT adi_sport_Open(
    const uint32_t              nDevNum,
    const ADI_SPORT_CHANNEL     eChannel,
    const ADI_SPORT_DIRECTION   eDirection,
    void                        *pMemory,
    const uint32_t              nMemSize,
    ADI_SPORT_HANDLE * const    phDevice
    );

/* Closes a SPORT device */
ADI_SPORT_RESULT adi_sport_Close(
    ADI_SPORT_HANDLE const      hDevice
    );

/* Submits a buffer to the driver */
ADI_SPORT_RESULT adi_sport_SubmitBuffer(
    ADI_SPORT_HANDLE const hDevice,
    void *           const pBuffer,
    uint32_t         const nNumBytes,
    bool             const bDMA
    );

/* Get the processed buffer from the driver */
ADI_SPORT_RESULT adi_sport_GetBuffer(
    ADI_SPORT_HANDLE const      hDevice,
    void ** const               ppBuffer,
    uint32_t *                  pHwError
    );

/* Peek function to know whether an processed buffer is avilable */
ADI_SPORT_RESULT adi_sport_IsBufferAvailable(
    ADI_SPORT_HANDLE const      hDevice,
    bool   * const              pbAvailable
    );

/* To register the callback function */
ADI_SPORT_RESULT adi_sport_RegisterCallback(
    ADI_SPORT_HANDLE const      hDevice,
    const ADI_CALLBACK          pfCallback,
    void * const                pCBparam
    );

/* Configure the data */
ADI_SPORT_RESULT adi_sport_ConfigData(
    ADI_SPORT_HANDLE  const             hDevice,
    const uint8_t                       nWordLength,
    const ADI_SPORT_PACKING_MODE        ePackMode,
    const bool                          bLSBFirst
    );

/* Configure the clock */
ADI_SPORT_RESULT adi_sport_ConfigClock(
    ADI_SPORT_HANDLE const      hDevice,
    const uint16_t              nClockRatio,
    const bool                  bUseIntlClock,
    const bool                  bRisingEdge,
    const bool                  bGatedClk
    );

/* Configure the frame sync */
ADI_SPORT_RESULT adi_sport_ConfigFrameSync(
    ADI_SPORT_HANDLE const      hDevice,
    const uint16_t              nFsDivisor,
    const bool                  bFSRequired,
    const bool                  bInternalFS,
    const bool                  bDataFS,
    const bool                  bActiveLowFS,
    const bool                  bLateFS,
    const bool                  bFSErrorOperation
    );

/* To mux the half-SPORT; this makes the device to use FS and Clock from other half-SPORT */
ADI_SPORT_RESULT adi_sport_MultiplexSportSignal(
    ADI_SPORT_HANDLE const      hDevice,
    const bool                  bUseOtherFS,
    const bool                  bUseOtherClk
    );

/* To configure the SPORT in timer mode */
ADI_SPORT_RESULT adi_sport_ConfigTimerMode(
    ADI_SPORT_HANDLE const      hDevice,
    const uint8_t               nFSDuration,
    const uint8_t               nWidth,
    const bool                  bActiveLow
    );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ADI_SPORT_H */
/*@}*/
