/*! *****************************************************************************
 * @file:    adi_crc_def.h
 * @brief:   Private header file for for CRC driver.
 * @details: This is a private header file for the CRC driver, which contains
 *           the API declarations, data and constant definitions used in driver
 *           implementation
 * -----------------------------------------------------------------------------
 * Copyright (c) 2010-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_CRC_DEF_H
#define ADI_CRC_DEF_H

/* CRC Driver includes */
#include <drivers/crc/adi_crc.h>

/*! \cond PRIVATE */

typedef struct __ADI_CRC_DEVICE ADI_CRC_DEVICE;
typedef ADI_CRC_RESULT (*CRC_BUFFER_SUBMIT) (ADI_CRC_DEVICE *pDevice, void *pBuffer, uint32_t NumBytes, uint32_t NumBits);

/* Enumeration of CRC operation status */
typedef enum
{
    ADI_CRC_OP_IDLE             = 0u,           /* CRC idle */
    ADI_CRC_OP_IN_PROGRESS      = 0x01u,        /* CRC operation in progress */
} ADI_CRC_OP_STATUS;

#pragma pack(push)
#pragma pack()

/* Structure to handle CRC Peripheral instance */
struct __ADI_CRC_DEVICE
{
    volatile ADI_CRC_TypeDef    *pReg;
    CRC_BUFFER_SUBMIT           pfSubmitBuffer;         /* Function for submitting CRC data buffer for calculation */
    ADI_CALLBACK                pfCallback;             /* Client supplied callback function */
    void                        *pCBParam;              /* Client supplied callback parameter */
    void                        *pRemainingData;        /* Pointer to the buffer containing remaining bytes */
    uint32_t                    RemainingBytes;         /* Remaining bytes */
    uint32_t                    RemainingBits;          /* Remaining bits */
    ADI_CRC_OP_STATUS           eCrcOpStatus;           /* Current status of the CRC Operation */
};

/* Structure to hold CRC device specific information */
typedef struct
{
    volatile ADI_CRC_TypeDef    *pReg;        /* CRC peripheral Registers */
    ADI_CRC_HANDLE              hDevice;      /* CRC device handle */
} ADI_CRC_INFO;

#pragma pack(pop)
/*! \endcond */

#endif  /* ADI_CRC_DEF_H */
