/*! ****************************************************************************
 @file:    adi_i2c_def.h
 @brief:   Internal I2C device driver definitions and macros
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2021 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_I2C_DEF_H
#define ADI_I2C_DEF_H

/*! \cond PRIVATE */

#include <drivers/i2c/adi_i2c.h>

#define ADI_I2C_NUM_INSTANCES       (1u)
#define ADI_I2C_STATUS_ERROR_MASK ( (1u << BITP_I2C_MSTAT_NACKADDR) \
                                  | (1u << BITP_I2C_MSTAT_NACKDATA) \
                                  | (1u << BITP_I2C_MSTAT_MRXOVR)   \
                                  | (1u << BITP_I2C_MSTAT_MTXUNDR)  \
                                  | (1u << BITP_I2C_MSTAT_ALOST)    )

/* Internal Actions */
static void           submitTransaction  (ADI_I2C_HANDLE const hDevice, ADI_I2C_TRANSACTION* const pTransaction);
static void           commenceTransmit   (ADI_I2C_HANDLE const hDevice);
static void           commenceReceive    (ADI_I2C_HANDLE const hDevice);
static ADI_I2C_RESULT i2cReset           (ADI_I2C_HANDLE const hDevice);

/* interrupt event handlers */
static void           transmitHandler    (ADI_I2C_HANDLE const hDevice);
static void           receiveHandler     (ADI_I2C_HANDLE const hDevice);
static void           completeHandler    (ADI_I2C_HANDLE const hDevice);
static void           errorHandler       (ADI_I2C_HANDLE const hDevice);

#if defined(__ADUCM302x__)
/*
 *  SYS Device Structure
 */
typedef struct _ADI_SYS_STRUCT
{
        ADI_SYS_TypeDef       *pReg;      /* Pointer to register base */
} ADI_SYS_STRUCT;

/* alias for the actual device structure */
typedef struct _ADI_SYS_STRUCT  ADI_SYS_REGISTERS;

#endif
/*
 *****************************************************************************
 * I2C Configuration structure.
 *****************************************************************************/
typedef struct __ADI_I2C_CONFIG {
    uint16_t MasterControlRegister;  /* I2C_MCTL register configuration.      */
    uint16_t ClockDividerRegister;   /* I2C_DIV  register.                    */
    uint16_t SharedControlRegister;  /* I2C_DIV  register.                    */
    uint16_t TimingControlRegister;  /* I2C_TCTL register.                    */
    uint16_t ClockStretchRegister;   /* I2C_ASTRETCH register.                */
    uint16_t TargetSlaveAddress;     /* slave address value (not a register). */
} ADI_I2C_CONFIG;


/* I2C physical device instance data */
typedef struct __ADI_I2C_DEVICE_INFO {
    IRQn_Type             pioIRQn;  /* PIO interrupt number                */
    ADI_I2C_TypeDef      *pDev;     /* pointer to i2c controller           */
    ADI_I2C_CONFIG       *pConfig;  /* pointer to user config info         */
    ADI_I2C_HANDLE        hDevice;  /* I2C handle or NULL if uninitialized */
} ADI_I2C_DEVICE_INFO;

/* I2C driver instance data structure */
typedef struct __ADI_I2C_DEV_DATA_TYPE {

    /* make sure to synchronize ANY size changes with ADI_I2C_MEMORY_SIZE macro in adi_i2c.h */

    /* device attributes */
    ADI_I2C_TypeDef      *pDev;
    ADI_I2C_DEVICE_INFO  *pDevInfo;


    /* driver state */
    uint16_t              hwStatus;
    bool                  bRepeatStart;
    uint16_t              i2cDeviceAddress;
    uint16_t              i2cEncodedDeviceAddress;    /* encoded as 7-bit device address + r/w LSB */
    bool                  bSubmitCalled;

    /* prologue data */
    volatile uint8_t     *pNextPrologueByte;
    volatile uint16_t     remainingPrologueCount;

    /* write data */
    volatile uint8_t     *pNextWriteByte;
    volatile uint16_t     remainingWriteCount;

    /* read data */
    volatile uint8_t     *pNextReadByte;
    volatile uint16_t     remainingReadCount;

    volatile uint16_t     nBytesProcessed;

    ADI_I2C_RESULT        result;         /* collector for return status */
    ADI_I2C_HW_ERRORS     hwErrors;       /* collector for error status  */
    ADI_I2C_HW_ERRORS     hwMaskedErrors; /* masked errors               */

    SEM_VAR_DECLR        /* blocking object: "Semaphore" for rtos,  "nLowPowerExitFlag" for non-rtos */

} ADI_I2C_DEV_DATA_TYPE;

/*! \endcond */

#endif    /* end of ifndef ADI_I2C_DEF_H */

