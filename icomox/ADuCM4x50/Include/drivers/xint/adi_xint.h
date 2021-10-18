/*! ****************************************************************************
 * @file:    adi_xint.h
 * @brief:   External interrupt driver definitions and API
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_XINT_H
#define ADI_XINT_H

/*! \addtogroup XINT_Driver External Interrupt Driver
 *  @{
 */

#ifdef __ICCARM__
#pragma diag_default=Pm008
#endif /* __ICCARM__ */

#include <adi_callback.h>
#include <adi_processor.h>

#if !defined(__ADUCM302x__) && !defined(__ADUCM4x50__)
#error "Unknown processor family"
#endif


/* C++ linkage */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! Amount of memory(in bytes) required by the External Interrupt device driver for its operation.
 * This memory is completely owned by the driver till the end of the operation.
 */
#define ADI_XINT_MEMORY_SIZE        (48u)

/*! External Interrupt Driver API function return codes */
typedef enum
{
    ADI_XINT_SUCCESS = 0,                       /*!< API successfully returned. */
    ADI_XINT_FAILURE,                           /*!< The API call failed. */
    ADI_XINT_ALREADY_INITIALIZED,               /*!< External interrupt driver has already been initialized. */
    ADI_XINT_NOT_INITIALIZED,                   /*!< External interrupt driver has not yet been initialized. */
    ADI_XINT_NULL_PARAMETER,                    /*!< The given pointer is pointing to NULL. */
    ADI_XINT_INVALID_MEMORY_SIZE,               /*!< The given memory is not sufficient to operate the driver. */   
    ADI_XINT_INVALID_INTERRUPT                  /*!< Invalid interrupt number. */
} ADI_XINT_RESULT;


/*! External interrupt trigger condition enumerations */
typedef enum {
    ADI_XINT_IRQ_RISING_EDGE    = 0x0,          /*!< Trigger an interrupt when a rising edge is detected.    */
    ADI_XINT_IRQ_FALLING_EDGE   = 0x1,          /*!< Trigger an interrupt when on a falling edge is detected.   */
    ADI_XINT_IRQ_EITHER_EDGE    = 0x2,          /*!< Trigger an interrupt on either falling or rising edge is detected. */
    ADI_XINT_IRQ_HIGH_LEVEL     = 0x3,          /*!< Trigger an interrupt on a logic level high is detected.     */
    ADI_XINT_IRQ_LOW_LEVEL      = 0x4           /*!< Trigger an interrupt on a logic level low is detected.  */
} ADI_XINT_IRQ_MODE;

/*! External interrupts. */
typedef enum {
    ADI_XINT_EVENT_INT0         = 0x0,          /*!< Event for external interrupt-0   */
    ADI_XINT_EVENT_INT1         = 0x1,          /*!< Event for external interrupt-1   */
    ADI_XINT_EVENT_INT2         = 0x2,          /*!< Event for external interrupt-2   */
    ADI_XINT_EVENT_INT3         = 0x3,          /*!< Event for external interrupt-3   */
    ADI_XINT_EVENT_RESERVED     = 0x4,          /*!< Event is reserved.   */
    ADI_XINT_EVENT_UART_RX      = 0x5,          /*!< Event for UART Rx activity   */
    ADI_XINT_EVENT_MAX          = 0x6           /*!< Number of external interrupt events */
} ADI_XINT_EVENT;


/* External Interrupt API functions */
ADI_XINT_RESULT  adi_xint_Init                  (void* const pMemory, uint32_t const MemorySize);
ADI_XINT_RESULT  adi_xint_UnInit                (void);
ADI_XINT_RESULT  adi_xint_EnableIRQ             (const ADI_XINT_EVENT eEvent, const ADI_XINT_IRQ_MODE eMode);
ADI_XINT_RESULT  adi_xint_DisableIRQ            (const ADI_XINT_EVENT eEvent); 
ADI_XINT_RESULT  adi_xint_RegisterCallback      (const ADI_XINT_EVENT eEvent, ADI_CALLBACK const pfCallback, void *const pCBParam );

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* ADI_XINT_H */
