/*! ****************************************************************************
 * @file    adi_data_transfer.h
 * @brief   General data transfer types for drivers
 * @details General data transfer types for drivers
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_DATA_TRANSFER_H
#define ADI_DATA_TRANSFER_H

/*=============  I N C L U D E S   =============*/

#include <stdint.h>                             /* defines types such as uint32_t*/
#include <rtos_map/adi_rtos_map.h>              /* needed for SEM_VAR_DECLR declaration */

/*! \cond PRIVATE */
/** @addtogroup Data_Transfer Common Data Transfer Structures
*  @{
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============  D E F I N E S  ===============*/

#define ADI_DT_BUFNUM   (2u)                    /*!< Number of buffers available for data transfers in each communication channel */

/*!
 *******************************************************************************
 * \struct ADI_DT_BUFF_INFO
 *  Structure for managing buffers submitted to drivers.
 ******************************************************************************/
struct _ADI_DT_BUFF_INFO;

/*!
 *******************************************************************************
 *  Structure for managing buffers submitted to drivers.
 ******************************************************************************/
typedef struct _ADI_DT_BUFF_INFO
{
    void *                      pStartAddress;  /*!< Address of buffer passed down a driver.            */
    uint32_t                    nCount;         /*!< Size of buffer in bytes.                           */
    uint32_t                    nIndex;         /*!< Position of first byte to be transmitted.          */
    bool                        bInUse;         /*!< Buffer in use flag.                                */
    bool                        bDMA;           /*!< Transaction is using the DMA flag.                 */
    struct _ADI_DT_BUFF_INFO *  pNextBuffer;    /*!< Pointer to the next buffer in the list.            */
} ADI_DT_BUFF_INFO;

/*!
 *******************************************************************************
 *  Enumeration of different data transfer modes supported by drivers.
 ******************************************************************************/
typedef enum _ADI_DT_MODE
{
    ADI_DT_MODE_NONE,           /*!< Mode of data transfer is not selected. */ 
    ADI_DT_MODE_BLOCKING,       /*!< Only calls to adi_xxx_Read or adi_xxx_Write are allowed for transferring data. */
    ADI_DT_MODE_NONBLOCKING     /*!< Only calls to adi_xxx_SubmitBuffer are allowed for transferring data. */
} ADI_DT_MODE;

typedef void * ADI_DEVICE_HANDLE;               /*!< Generic device handle */

/*!
 *******************************************************************************
 *  Structure for managing pool of buffers submitted to drivers.
 ******************************************************************************/
typedef struct
{
    ADI_DT_BUFF_INFO    BufInfo[ADI_DT_BUFNUM]; /*!< Ping Pong Buffers.                                             */
    ADI_DT_BUFF_INFO *  pFreeBuffer;            /*!< Pointer to free buffer. (Next buffer to submit).               */
    ADI_DT_BUFF_INFO *  pFillBuffer;            /*!< Pointer to the next buffer to be filled. (Needed for the case
                                                     where many buffers are "submitted" before a "get" is called.)  */
    ADI_DT_BUFF_INFO *  pActiveBuffer;          /*!< Pointer to active buffer. (Next buffer waiting for completion.)*/
    ADI_DT_MODE         eDataTranferMode;       /*!< Data transfer mode (blocking or non-blockig).                  */

    SEM_VAR_DECLR
} ADI_DT_CHANNEL;


/*=============  P U B L I C   F U N C T I O N S =============*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

/*! \endcond */

#endif /* ADI_DATA_TRANSFER_H */
/*@}*/
