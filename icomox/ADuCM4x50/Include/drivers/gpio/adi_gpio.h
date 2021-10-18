/*! ****************************************************************************
 * @file:    adi_gpio.h
 * @brief:   GPIO definitions and API
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_GPIO_H
#define ADI_GPIO_H

#include <adi_processor.h>
#include <adi_callback.h>

#ifdef __ICCARM__
/* IAR MISRA C 2004 error suppressions.
 *
 * Pm008 (rule 2.4): sections of code should not be 'commented out'.
 *   Allow code example in doxygen comment.
 * Pm011 (rule 6.3): The basic types of char, int, long, float cannot be used.
 *   bool is used in the APIs as it is not affending the rule. Disabling this as IAR treats it as an error.
 */
#pragma diag_suppress=Pm008,Pm011
#endif /* __ICCARM__ */

/*! \addtogroup GPIO_Driver GPIO Driver
 *  @{
 */

#ifdef __ICCARM__
#pragma diag_default=Pm008
#endif /* __ICCARM__ */

/* C++ linkage */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*! Amount of memory(in bytes) required by the GPIO device driver for its operation.
 * This memory is completely owned by the driver till the end of the operation.
 */
#define ADI_GPIO_MEMORY_SIZE        (16u)

/* typedefs for 16-bit Ports */
typedef uint16_t ADI_GPIO_DATA;    /*!< pin data reg type */


/*! GPIO API function return codes */
typedef enum
{
    ADI_GPIO_SUCCESS = 0,                   /*!< No error detected. */
    ADI_GPIO_FAILURE,                       /*!< The API call failed. */
    ADI_GPIO_ALREADY_INITIALIZED,           /*!< GPIO device has already been initialized. */
    ADI_GPIO_NOT_INITIALIZED,               /*!< GPIO device has not yet been initialized. */
    ADI_GPIO_NULL_PARAMETER,                /*!< The given pointer is pointing to NULL. */
    ADI_GPIO_INVALID_MEMORY_SIZE,           /*!< The given memory is not sufficient to operate the driver. */
    ADI_GPIO_INVALID_PINS,                  /*!< Invalid pin combination. */
    ADI_GPIO_INVALID_INTERRUPT,             /*!< Invalid interrupt number. */
    ADI_GPIO_INVALID_TRIGGER,               /*!< Invalid trigger condition. */
    ADI_GPIO_INVALID_PORT,                  /*!< Invalid port number. */
    
} ADI_GPIO_RESULT;


/*! GPIO trigger condition enumerations */
typedef enum {
    ADI_GPIO_IRQ_RISING_EDGE    =(0x0),     /*!< Trigger an interrupt on a rising edge.    */
    ADI_GPIO_IRQ_FALLING_EDGE   =(0x1),     /*!< Trigger an interrupt on a falling edge.   */
    ADI_GPIO_IRQ_EITHER_EDGE    =(0x2),     /*!< Trigger an interrupt on either edge.      */
    ADI_GPIO_IRQ_HIGH_LEVEL     =(0x3),     /*!< Trigger an interrupt on a high level.     */
    ADI_GPIO_IRQ_LOW_LEVEL      =(0x4)      /*!< Trigger an interrupt on a low level.      */
} ADI_GPIO_IRQ_TRIGGER_CONDITION;

/*! GPIO IRQ enumeration */
typedef enum {
    ADI_GPIO_INTA_IRQ = SYS_GPIO_INTA_IRQn, /*!< GPIO Group Interrupt A.    */
    ADI_GPIO_INTB_IRQ = SYS_GPIO_INTB_IRQn, /*!< GPIO Group Interrupt B.   */
} ADI_GPIO_IRQ;


/*! GPIO port enumerations */
typedef enum {
    ADI_GPIO_PORT0,      /*!< Port 0 */
    ADI_GPIO_PORT1,      /*!< Port 1 */
    ADI_GPIO_PORT2,      /*!< Port 2 */
#if defined(__ADUCM4x50__)
    ADI_GPIO_PORT3,      /*!< Port 3 */
#endif /* __ADUCM4x50__ */
    ADI_GPIO_NUM_PORTS   /*!< maximum number of ports */
} ADI_GPIO_PORT;

/* 16-bit port pin defs */
#define ADI_GPIO_PIN_0      ((ADI_GPIO_DATA)(0x0001)) /*!< Pin 0 */
#define ADI_GPIO_PIN_1      ((ADI_GPIO_DATA)(0x0002)) /*!< Pin 1 */
#define ADI_GPIO_PIN_2      ((ADI_GPIO_DATA)(0x0004)) /*!< Pin 2 */
#define ADI_GPIO_PIN_3      ((ADI_GPIO_DATA)(0x0008)) /*!< Pin 3 */
#define ADI_GPIO_PIN_4      ((ADI_GPIO_DATA)(0x0010)) /*!< Pin 4 */
#define ADI_GPIO_PIN_5      ((ADI_GPIO_DATA)(0x0020)) /*!< Pin 5 */
#define ADI_GPIO_PIN_6      ((ADI_GPIO_DATA)(0x0040)) /*!< Pin 6 */
#define ADI_GPIO_PIN_7      ((ADI_GPIO_DATA)(0x0080)) /*!< Pin 7 */
#define ADI_GPIO_PIN_8      ((ADI_GPIO_DATA)(0x0100)) /*!< Pin 8 */
#define ADI_GPIO_PIN_9      ((ADI_GPIO_DATA)(0x0200)) /*!< Pin 9 */
#define ADI_GPIO_PIN_10     ((ADI_GPIO_DATA)(0x0400)) /*!< Pin 10 */
#define ADI_GPIO_PIN_11     ((ADI_GPIO_DATA)(0x0800)) /*!< Pin 11 */
#define ADI_GPIO_PIN_12     ((ADI_GPIO_DATA)(0x1000)) /*!< Pin 12 */
#define ADI_GPIO_PIN_13     ((ADI_GPIO_DATA)(0x2000)) /*!< Pin 13 */
#define ADI_GPIO_PIN_14     ((ADI_GPIO_DATA)(0x4000)) /*!< Pin 14 */
#define ADI_GPIO_PIN_15     ((ADI_GPIO_DATA)(0x8000)) /*!< Pin 15 */

/* GPIO port pins availability mask */
#define ADI_GPIO_PORT0_PIN_AVL    (0xFFFFu)           /*!< Port 0 pin mask (16 pins)*/
#define ADI_GPIO_PORT1_PIN_AVL    (0xFFFFu)           /*!< Port 1 pin mask (16 pins)*/
#define ADI_GPIO_PORT2_PIN_AVL    (0xFFFFu)           /*!< Port 2 pin mask (16 pins)*/

#if defined(__ADUCM4x50__)
#define ADI_GPIO_PORT3_PIN_AVL    (0x000Fu)           /*!< Port 2 pin mask (4 pins) */
#endif /* __ADUCM4x50__ */

/* GPIO API functions */
ADI_GPIO_RESULT  adi_gpio_Init                         (void* const pMemory, uint32_t const MemorySize);
ADI_GPIO_RESULT  adi_gpio_UnInit                       (void);
ADI_GPIO_RESULT  adi_gpio_RegisterCallback             (const ADI_GPIO_IRQ eIrq, ADI_CALLBACK const pfCallback, void *const pCBParam );
ADI_GPIO_RESULT  adi_gpio_SetGroupInterruptPins        (const ADI_GPIO_PORT Port, const ADI_GPIO_IRQ eIrq, const ADI_GPIO_DATA Pins);
ADI_GPIO_RESULT  adi_gpio_GetGroupInterruptPins        (const ADI_GPIO_PORT Port, const ADI_GPIO_IRQ eIrq, ADI_GPIO_DATA* const pValue);
ADI_GPIO_RESULT  adi_gpio_SetGroupInterruptPolarity    (const ADI_GPIO_PORT Port, const ADI_GPIO_DATA Pins);
ADI_GPIO_RESULT  adi_gpio_GetGroupInterruptPolarity    (const ADI_GPIO_PORT Port, ADI_GPIO_DATA* const pValue);
ADI_GPIO_RESULT  adi_gpio_GroupInterruptPolarityEnable (const ADI_GPIO_PORT Port, const ADI_GPIO_DATA Pins, const bool bFlag);
ADI_GPIO_RESULT  adi_gpio_OutputEnable                 (const ADI_GPIO_PORT Port, const ADI_GPIO_DATA Pins, const bool bFlag);
ADI_GPIO_RESULT  adi_gpio_GetOutputEnable              (const ADI_GPIO_PORT Port, ADI_GPIO_DATA* const pValue);
ADI_GPIO_RESULT  adi_gpio_InputEnable                  (const ADI_GPIO_PORT Port, const ADI_GPIO_DATA Pins, const bool bFlag);
ADI_GPIO_RESULT  adi_gpio_PullUpEnable                 (const ADI_GPIO_PORT Port, const ADI_GPIO_DATA Pins, const bool bFlag);
ADI_GPIO_RESULT  adi_gpio_DriveStrengthEnable          (const ADI_GPIO_PORT Port, const ADI_GPIO_DATA Pins, const bool bFlag);
ADI_GPIO_RESULT  adi_gpio_SetHigh                      (const ADI_GPIO_PORT Port, const ADI_GPIO_DATA Pins);
ADI_GPIO_RESULT  adi_gpio_SetLow                       (const ADI_GPIO_PORT Port, const ADI_GPIO_DATA Pins);
ADI_GPIO_RESULT  adi_gpio_Toggle                       (const ADI_GPIO_PORT Port, const ADI_GPIO_DATA Pins);
ADI_GPIO_RESULT  adi_gpio_SetData                      (const ADI_GPIO_PORT Port, const ADI_GPIO_DATA Pins);
ADI_GPIO_RESULT  adi_gpio_GetData                      (const ADI_GPIO_PORT Port, const ADI_GPIO_DATA Pins, uint16_t* const pValue);
ADI_GPIO_RESULT  adi_gpio_GetOutputData                (const ADI_GPIO_PORT Port, ADI_GPIO_DATA* const pValue);
#if defined (__ICCARM__)
#pragma diag_default=Pm011
#endif

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* ADI_GPIO_V1_H */
