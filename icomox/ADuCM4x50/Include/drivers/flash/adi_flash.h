/*! ****************************************************************************
 * @file:    adi_flash.h
 * @brief:   Flash device driver definitions
 * -----------------------------------------------------------------------------
 * Copyright (c) 2012-2020 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/

/*! @addtogroup Flash_Driver Flash Driver
 *  @{
 */

#ifndef ADI_FLASH_H
#define ADI_FLASH_H

 /*! \cond PRIVATE */
#include <adi_processor.h>
#include <adi_callback.h>
#include <rtos_map/adi_rtos_map.h>  /* for ADI_SEM_SIZE */
/*! \endcond */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * \enum ADI_FEE_RESULT
 * Flash Controller return codes.
 */
 typedef enum {
    ADI_FEE_SUCCESS = 0,                /*!< The function completed successfully.               */
    ADI_FEE_ERR_ALIGNMENT,              /*!< The flash write source data pointer is misaligned. */
    ADI_FEE_ERR_ALREADY_INITIALIZED,    /*!< The flash device driver is already initialized.    */
    ADI_FEE_ERR_BAD_DEVICE_NUM,         /*!< Device number passed is invalid.                   */
    ADI_FEE_ERR_BUFFER_ERR,             /*!< An error occurred while processing a write buffer. */
    ADI_FEE_ERR_DEVICE_BUSY,            /*!< The device is busy.                                */
    ADI_FEE_ERR_DMA_BUS_FAULT,          /*!< Runtime DMA bus fault detected.                    */
    ADI_FEE_ERR_DMA_INVALID_DESCR,      /*!< Runtime DMA invalid descriptor detected.           */
    ADI_FEE_ERR_DMA_REGISTER,           /*!< Error registering DMA error callback function.     */
    ADI_FEE_ERR_DMA_UNKNOWN_ERROR,      /*!< Unknown runtime DMA error detected.                */
    ADI_FEE_ERR_HW_ERROR_DETECTED,      /*!< An FEE hardware error occurred (pHwErrors param).  */
    ADI_FEE_ERR_INSUFFICIENT_MEM,       /*!< The memory passed is undersized.                   */
    ADI_FEE_ERR_INVALID_HANDLE,         /*!< Device Handle is invalid.                          */
    ADI_FEE_ERR_INVALID_PARAM,          /*!< A function parameter is invalid.                   */
    ADI_FEE_ERR_NO_DATA_TO_TRANSFER,    /*!< No transfer data detected.                         */
    ADI_FEE_ERR_TRANSFER_IN_PROGRESS,   /*!< Operation already in progress.                     */
    ADI_FEE_ERR_UNMATCHED_SUBMIT_QUERY, /*!< Unmatched read/write vs. submit/get API call.      */
    ADI_FEE_ERR_SEMAPHORE_FAILED,       /*!< An semaphore operation failed.                     */
 } ADI_FEE_RESULT;


/*! A device handle used in all API functions to identify the flash device. */
typedef struct __ADI_FEE_DEV_DATA_TYPE* ADI_FEE_HANDLE;


/*! Applications use the "ADI_FEE_MEMORY_SIZE" macro to allocate
    required flash driver memory.  This memory (and size) are passed
    to the flash driver during the "adi_fee_Open()" driver initialization
    call.  This memory is used to store internal flash driver state.
*/
#define ADI_FEE_MEMORY_SIZE            (44u + ADI_SEM_SIZE)

/*! Number of flash controller instances */
#define ADI_FEE_NUM_INSTANCES       (1u)

#if defined (__ADUCM4x50__)
#define FEE_FLASH_SIZE     (0x80000u)   /*!< Flash memory size is 512kB total */
#define FEE_BLOCK_SHIFT    (14u)        /*!< 16kB block size */
#elif defined (__ADUCM302x__)
#define FEE_FLASH_SIZE     (0x40000u)   /*!< Flash memory size is 256kB total */
#define FEE_BLOCK_SHIFT    (13u)        /*!< 8kB block size */
#else
#error processor not supported
#endif

/**
 * Flash Size and Page/Block macros:
 *
 * ADuCM302x
 * - 256kB total user space, broken up as
 * - 128-pages, 2kB/page
 * - 32-blocks, 8kB/block
 * - 8 pages/block
 *
 * ADuCM4x50
 * - 512kB total user space, broken up as
 * - 256-pages, 2kB/page
 * - 32-blocks, 16kB/block
 * - 8 pages/block
 */

#define FEE_PAGE_SHIFT     (11u)                               /*!< 2kB page size (2^11)*/
#define FEE_MAX_NUM_PAGES  (FEE_FLASH_SIZE >> FEE_PAGE_SHIFT)  /*!< max number of pages */
#define FEE_MAX_NUM_BLOCKS (FEE_FLASH_SIZE >> FEE_BLOCK_SHIFT) /*!< max number of blocks (32) */


/*!
 * \enum ADI_FEE_CALLBACK_EVENT
 * Enum for the callback events.
 */
typedef enum {
     ADI_FEE_CALLBACK_EVENT_BUFFER_PROCESSED,    /*!< Buffer processed successfully event. */
     ADI_FEE_CALLBACK_EVENT_DEVICE_ERROR,        /*!< Device error(s) detected during command. */
} ADI_FEE_CALLBACK_EVENT;

/*!
 * \enum ADI_FEE_ECC_EVENT_TYPE
 * Enum for the Error-Correction-Code event type.
 */
typedef enum {
    ADI_FEE_ECC_EVENT_TYPE_ERROR,       /*!< ECC Error Event. */
    ADI_FEE_ECC_EVENT_TYPE_CORRECT      /*!< ECC correction event. */
} ADI_FEE_ECC_EVENT_TYPE;

/*!
 * \enum ADI_FEE_ECC_RESPONSE
 * Error-Correction-Code configuration codes.
 */
typedef enum {
    ADI_FEE_ECC_RESPONSE_NONE       = 0x0,      /*!< No Response. */
    ADI_FEE_ECC_RESPONSE_BUS_ERROR  = 0x1,      /*!< Generate a Bus Error. */
    ADI_FEE_ECC_RESPONSE_IRQ        = 0x2       /*!< Generate an IRQ. */
} ADI_FEE_ECC_RESPONSE;


/*!
 * \struct ADI_FEE_TRANSACTION
 * Flash write data transaction block.
 */
typedef struct {
    uint32_t   *pWriteAddr;  /*!< Pointer to flash-space (destination) write location. */
    uint32_t   *pWriteData;  /*!< Pointer to user-space (source) write Data.          */
    uint32_t    nSize;       /*!< Write data size (in bytes).                          */
    bool        bUseDma;     /*!< DMA flag controlling use of DMA or not.              */
} ADI_FEE_TRANSACTION;


/*================ E X T E R N A L S ==================*/
/* Flash Controller API */

ADI_FEE_RESULT adi_fee_Open              (uint32_t const nDeviceNum, void* const pMemory, uint32_t const nMemorySize, ADI_FEE_HANDLE* const phDevice);
ADI_FEE_RESULT adi_fee_Close             (ADI_FEE_HANDLE const hDevice);
ADI_FEE_RESULT adi_fee_RegisterCallback  (ADI_FEE_HANDLE const hDevice, ADI_CALLBACK const pfCallback, void* const pCBParam);

ADI_FEE_RESULT adi_fee_PageErase         (ADI_FEE_HANDLE const hDevice, uint32_t const nPageNumStart, uint32_t const nPageNumEnd, uint32_t* const pHwErrors);
ADI_FEE_RESULT adi_fee_MassErase         (ADI_FEE_HANDLE const hDevice, uint32_t* const pHwErrors);

ADI_FEE_RESULT adi_fee_Write             (ADI_FEE_HANDLE const hDevice, ADI_FEE_TRANSACTION* const pTransaction, uint32_t* const pHwErrors);
ADI_FEE_RESULT adi_fee_SubmitBuffer      (ADI_FEE_HANDLE const hDevice, ADI_FEE_TRANSACTION* const pTransaction);

ADI_FEE_RESULT adi_fee_IsBufferAvailable (ADI_FEE_HANDLE const hDevice, bool* const pbCompletionState);
ADI_FEE_RESULT adi_fee_GetBuffer         (ADI_FEE_HANDLE const hDevice, uint32_t* const pHwErrors);

ADI_FEE_RESULT adi_fee_GetPageNumber     (ADI_FEE_HANDLE const hDevice, uint32_t const nAddress, uint32_t* const pnPageNum);
ADI_FEE_RESULT adi_fee_GetBlockNumber    (ADI_FEE_HANDLE const hDevice, uint32_t const nAddress, uint32_t* const pnBlockNum);

ADI_FEE_RESULT adi_fee_VerifySignature   (ADI_FEE_HANDLE const hDevice, uint32_t const nStartPage, uint32_t const nEndPage, uint32_t* const pSigResult, uint32_t* const pHwErrors);
ADI_FEE_RESULT adi_fee_WriteProtectBlock (ADI_FEE_HANDLE const hDevice, uint32_t const  nBlockNum);

ADI_FEE_RESULT adi_fee_Sleep             (ADI_FEE_HANDLE const hDevice, bool const bSleep);
ADI_FEE_RESULT adi_fee_Abort             (ADI_FEE_HANDLE const hDevice);
ADI_FEE_RESULT adi_fee_GetAbortAddr      (ADI_FEE_HANDLE const hDevice, uint32_t* const pnAddress);

ADI_FEE_RESULT adi_fee_ConfigECC         (ADI_FEE_HANDLE const hDevice, uint32_t const nStartPage, bool const bInfoECCEnable);
ADI_FEE_RESULT adi_fee_EnableECC         (ADI_FEE_HANDLE const hDevice, bool const bEnable);
ADI_FEE_RESULT adi_fee_ConfigECCEvents   (ADI_FEE_HANDLE const hDevice, ADI_FEE_ECC_EVENT_TYPE const eEvent, ADI_FEE_ECC_RESPONSE const eResponse);
ADI_FEE_RESULT adi_fee_GetECCErrAddr     (ADI_FEE_HANDLE const hDevice, uint32_t* const pnAddress);
ADI_FEE_RESULT adi_fee_GetECCCorrections (ADI_FEE_HANDLE const hDevice, uint32_t* const pnNumCorrections);

#if defined (__ADUCM4x50__) 
ADI_FEE_RESULT adi_fee_ConfigureWaitStates (ADI_FEE_HANDLE const hDevice, uint8_t numWaitStates);
#endif

#ifdef __cplusplus
}
#endif

#endif /* include guard */

/*
** EOF
*/

/*@}*/
