/*! ****************************************************************************
 * @file:    adi_rtos_map_freertos.h
 * @brief:   FreeRTOS RTOS API mapping file.
 *           This file maps the RTOS macros to FreeRTOS APIs
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2020 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_RTOS_MAP_FREERTOS_H
#define ADI_RTOS_MAP_FREERTOS_H

/* If building a c file */
#if defined(__STDC__)

#include <FreeRTOS.h>
#include "semphr.h"

extern BaseType_t xHigherPriorityTaskWoken;
  
/*! Macro that declares the semaphore type that the drivers use. 
    The macro should be used within the device data structure.
    It should not be used to declare the semaphore as a global variable. */
#define SEM_VAR_DECLR                                                                    \
                            StaticQueue_t hSemaphore;                                  
                            
/*! Memory required for semaphore in terms bytes. This size is used to compute 
    the total memory required for the operation of the driver. FreeRtos does not 
    require semaphore memory to be passed by application. But memory is required
    to store the handle. */
#define ADI_SEM_SIZE        (sizeof(StaticQueue_t))                            

/*! Macro that creates a semaphore and returns the error specified in case of failure. DEV is the handle to the device driver structure that contains the semaphore/semaphore handle. */

                        /*! Macro that creates a semaphore and returns the error specified in case of failure. DEV is the handle to the device driver structure that contains the semaphore/semaphore handle. */
#define SEM_CREATE(DEV, name, error)                                                     \
                   do {                                                                  \
                         xSemaphoreCreateBinaryStatic(&(DEV)->hSemaphore);               \
                      } while (0)

/*! Macro that deletes a semaphore and returns the error specified in case of failure. DEV is the handle to the device driver structure that contains the semaphore/semaphore handle. */
#define SEM_DELETE(DEV, error)                                                           \
                   do {                                                                  \
                            vSemaphoreDelete (&(DEV)->hSemaphore);                       \
                      } while (0)


/*! Macro that blocks indefinitely on a semaphore and returns error in case of failure. DEV is the handle to the device driver structure that contains the semaphore handle.*/
#define SEM_PEND(DEV, error)                                                                                \
                 do {                                                                                       \
                       if(xSemaphoreTake ((SemaphoreHandle_t)&(DEV)->hSemaphore, portMAX_DELAY) != pdTRUE)  \
                       return((error));                                                                     \
                    } while (0)

/*! Macro that posts a semaphore. DEV is the handle to the device driver structure that contains the semaphore handle. */
/* Note that priority inversion is supported */
#define SEM_POST(DEV)                                                                     \
        do {                                                                              \
          /* Assume that a higher priority task can be schedule in */                     \
          BaseType_t xHigherPriorityTaskWoken = pdTRUE;                                   \
          xSemaphoreGiveFromISR(&(DEV)->hSemaphore, &xHigherPriorityTaskWoken);           \
        } while (0)                        

/*! Defines a local variable where interrupt status register value is stored. 
    This macro should be used within a function in which critical section
    macros ADI_ENTER_CRITICAL_REGION and ADI_EXIT_CRITICAL_REGION are 
    used. 
    
    @sa ADI_ENTER_CRITICAL_REGION()
    @sa ADI_EXIT_CRITICAL_REGION()
    */
#define ADI_INT_STATUS_ALLOC()      uint32_t IntStatus = 0u

/*! Macro to enter critical section. To use this macro, the
    interrupt status variable should be defined (ADI_INT_STATUS_ALLOC)
    in the same scope. 
    
    @sa ADI_INT_STATUS_ALLOC()
*/
#define ADI_ENTER_CRITICAL_REGION()                                                \
do {                                                                               \
      IntStatus  = __get_PRIMASK();                                                \
    __disable_irq();                                                               \
} while (0)

/*! Macro to exit critical section.To use this macro, the
    interrupt status variable should be defined (ADI_INT_STATUS_ALLOC)
    in the same scope. 

    @sa  ADI_INT_STATUS_ALLOC()   
*/ 
#define ADI_EXIT_CRITICAL_REGION()                                                 \
do {                                                                               \
  __set_PRIMASK(IntStatus);                                                        \
} while (0)

/*! Code that uCOS requires to be run in the beginning of an interrupt handler. 
    @sa ISR_EPILOG()
*/
#define ISR_PROLOG()            

/*! Code that uCOS requires to be run in the end of an interrupt handler. 
    @sa ISR_PROLOG()
*/
#define ISR_EPILOG()   portYIELD()         
     
#endif /* __STDC__  */

#define PENDSV_HANDLER   xPortPendSVHandler
#define SYSTICK_HANDLER  xPortSysTickHandler
#define SVC_HANDLER      vPortSVCHandler

#ifndef ADI_MAX_IRQ_PRIORITY
/* For FreeRTOS we do not want to use the default priorities for IRQ interrupts. */
#define ADI_MAX_IRQ_PRIORITY  (1u)
#endif

#endif /* ADI_RTOS_MAP_FREERTOS_H */
