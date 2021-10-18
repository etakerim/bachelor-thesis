/*! ****************************************************************************
 * @file    adi_wdt.h
 * @brief   WDT device driver public header
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/


#ifndef ADI_WDT_H
#define ADI_WDT_H

#include <adi_callback.h>

/** @addtogroup WDT_Driver WDT Driver
 *  @{
 */

/* C++ linkage */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! \enum ADI_WDT_RESULT Watchdog Device Error Codes. */
typedef enum
{
    /*! Generic success. */
    ADI_WDT_SUCCESS,
    /*! Timer is locked. */
    ADI_WDT_FAILURE_LOCKED
} ADI_WDT_RESULT;


/******************************************************************************
 * PUBLIC API
 * 1.) Eliminatable functions that may be optimized out by the linker
 * 2.) Ordered by designed function call sequence
 *****************************************************************************/

ADI_WDT_RESULT adi_wdt_Enable  (bool const bEnable, ADI_CALLBACK const pfCallback);
void           adi_wdt_Kick    (void);
void           adi_wdt_GetCount(uint16_t * const pCurCount);


#ifdef __cplusplus
}
#endif


/*! @} */

#endif /* ADI_WDT_H */

