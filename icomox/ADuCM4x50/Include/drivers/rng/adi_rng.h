/*! ****************************************************************************
 @file    adi_rng.h
 @brief   Random Number Generator Driver
 * -----------------------------------------------------------------------------
 * Copyright (c) 2012-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/

/*! \addtogroup RNG_Driver RNG Driver
 *  Random Number Generator Driver
 *  @{
 */

#ifndef ADI_RNG_H
#define ADI_RNG_H

#include <adi_processor.h>
#include <adi_callback.h>

#if  !defined(__ADUCM4x50__) &&  !defined(__ADUCM302x__)
#error "Unsupported processor"
#endif

#include <adi_rng_config.h>

#ifdef __ICCARM__
/* IAR MISRA C 2004 error suppressions.
 *
 * Pm011 (rule 6.3): The basic types of char, int, long, float cannot be used.
 *   bool is used in the APIs as it is not affending the rule. Disabling this as IAR treats it as an error.
 */
#pragma diag_suppress=Pm011
#endif /* __ICCARM__ */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * \enum ADI_RNG_RESULT
 * Random Number Generator API return codes
 */
typedef enum
{
    ADI_RNG_SUCCESS = 0,            /*!<  No Error, API suceeded */
    ADI_RNG_UNKNOWN_ERROR,          /*!<  Unknown error detected */
    ADI_RNG_ALREADY_INITIALIZED,    /*!<  RNG is already initialized */
    ADI_RNG_INVALID_PARAM,          /*!<  Invalid function parameter */
    ADI_RNG_BAD_DEV_HANDLE,         /*!<  Invalid device handle passed */
    ADI_RNG_BAD_DEVICE_NUM,         /*!<  Invalid device instance */
    ADI_RNG_NOT_INITIALIZED,        /*!<  RNG not yet initialized */
    ADI_RNG_INVALID_STATE           /*!<  Device is in an invalid state */
} ADI_RNG_RESULT;

/*!
 * \enum ADI_RNG_EVENT
 * Random Number Generator callback events
 */
typedef enum
{
    ADI_RNG_EVENT_READY,               /*!< Random number ready event */
    ADI_RNG_EVENT_STUCK                /*!< The ring oscillator got stuck event */
} ADI_RNG_EVENT;


/*! The amount of application supplied memory required by the RNG driver */
#define ADI_RNG_MEMORY_SIZE        (12u)


/*! RNG Device handle typedef */
typedef void* ADI_RNG_HANDLE;

/*================ E X T E R N A L S ==================*/

/*
 * RNG API
 */

/* Open a random number generator device */
extern ADI_RNG_RESULT adi_rng_Open(
                            uint32_t             const  nDeviceNum,
                            void*                const  pMemory,
                            uint32_t             const  MemorySize,
                            ADI_RNG_HANDLE*      const  phDevice
                            );

/* Close the RNG Device */
extern ADI_RNG_RESULT adi_rng_Close(ADI_RNG_HANDLE hDevice);

/* Enable/Disable the device */
extern ADI_RNG_RESULT adi_rng_Enable (
                                      ADI_RNG_HANDLE const hDevice,
                                      bool           const bFlag
                                      );
/* Enable/Disable buffering */
extern ADI_RNG_RESULT adi_rng_EnableBuffering (
                                               ADI_RNG_HANDLE const hDevice,
                                               bool           const bFlag
                                               );

/* Set the sample length */
extern ADI_RNG_RESULT adi_rng_SetSampleLen (
                                            ADI_RNG_HANDLE const hDevice,
                                            uint16_t       const nLenPrescaler,
                                            uint16_t       const nLenReload
                                            );

/* Get whether the random number is ready */
extern ADI_RNG_RESULT adi_rng_GetRdyStatus (
                                            ADI_RNG_HANDLE const hDevice,
                                            bool*          const pbFlag
                                            );

/* Get whether the ring oscillator output is stuck or not */
extern ADI_RNG_RESULT adi_rng_GetStuckStatus (
                                              ADI_RNG_HANDLE const hDevice,
                                              bool*          const pbFlag
                                              );

/* Get the random number */
extern ADI_RNG_RESULT adi_rng_GetRngData (
                                          ADI_RNG_HANDLE const hDevice,
                                          uint32_t*      const pRegData
                                          );

/* Get the oscillator count */
extern ADI_RNG_RESULT adi_rng_GetOscCount (
                                           ADI_RNG_HANDLE const hDevice,
                                           uint32_t*      const pOscCount
                                           );

/* Get the oscillator count difference value */
extern ADI_RNG_RESULT adi_rng_GetOscDiff (
                                          ADI_RNG_HANDLE const hDevice,
                                          uint32_t       const nIndex,
                                          uint8_t*       const pOscDiff
                                          );

/* Register a callback */
extern ADI_RNG_RESULT adi_rng_RegisterCallback (
                                                ADI_RNG_HANDLE  hDevice,
                                                ADI_CALLBACK    cbFunc,
                                                void           *pCBParam
                                                );

/* Retrieve the current RNG sample length prescale and reload value configured in the device. */
extern ADI_RNG_RESULT adi_rng_GetSampleLen (
                                            ADI_RNG_HANDLE const hDevice,
                                            uint16_t*      const pLenPrescaler,
                                            uint16_t*      const pLenReload
                                            );

#ifdef __cplusplus
}
#endif

#ifdef __ICCARM__
#pragma diag_default=Pm011
#endif /* __ICCARM__ */
#endif /* include guard */

/*
** EOF
*/

/*@}*/
/*@}*/
