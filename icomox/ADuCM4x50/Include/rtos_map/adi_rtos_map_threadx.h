/*! ****************************************************************************
 * @file:    adi_rtos_map_threadx.h
 * @brief:   THREADX RTOS API mapping file.
 *           This file maps the RTOS macros to THREADX APIs
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_RTOS_MAP_THREADX_H
#define ADI_RTOS_MAP_THREADX_H

/* If building a c file */
#if defined(__STDC__)


#include <tx_api.h>
#include <stdint.h>
#include <stddef.h>

/*! Macro that declares the semaphore type that the drivers use. 
    The macro should be used within the device data structure.
    It should not be used to declare the semaphore as a global variable. */
#define SEM_VAR_DECLR                                                                          \
                    TX_SEMAPHORE Semaphore;                                  

/*! Memory required for semaphore in bytes. This size is used to compute 
    the total memory required for the operation of the driver. ThreadX requires
    semaphore memory to be passed by application. But there is no memory required
    to store the handle. For every semaphore related call the same memory pointer 
    that was used during create will be passed. */
#define ADI_SEM_SIZE        (sizeof(TX_SEMAPHORE))
                                        
/*! Macro that creates a semaphore and returns the error specified in case of failure. DEV is the handle to the device driver structure that contains the semaphore/semaphore handle. */

#define SEM_CREATE(DEV, name, error)                                                           \
                   do {                                                                        \
                         UINT    status;                                                       \
                         status = tx_semaphore_create((&(DEV)->Semaphore),name,0);             \
                         if(TX_SUCCESS != status) {return((error));}                           \
                      } while (0)


/*! Macro that deletes a semaphore and returns the error specified in case of failure. DEV is the handle to the device driver structure that contains the semaphore/semaphore handle. */

#define SEM_DELETE(DEV, error)                                                                 \
                   do {                                                                        \
                         UINT    status;                                                       \
                         status = tx_semaphore_delete(&((DEV)->Semaphore));                    \
                         if(TX_SUCCESS != status) {return((error));}                           \
                      } while (0)                        
                                                                                


/*! Macro that blocks indefinitely on a semaphore and returns error in case of failure. DEV is the handle to the device driver structure that contains the semaphore handle.*/
#define SEM_PEND(DEV, error)                                                                   \
        do {                                                                                   \
              UINT    status;                                                                  \
              status=tx_semaphore_get(&((DEV)->Semaphore), TX_WAIT_FOREVER);                   \
              if(TX_SUCCESS != status) {return((error));}                                      \
           } while (0)                        
                        


               
/*! Macro that posts a semaphore. DEV is the handle to the device driver structure that contains the semaphore handle. */
#define SEM_POST(DEV)                                                                          \
        do {                                                                                   \
             tx_semaphore_put(&((DEV)->Semaphore));                                            \
        } while (0)             
             

          

/*! Local variable where interrupt status register value is stored. 
    This macro should be used within a function in which critical section
    macros ADI_ENTER_CRITICAL_REGION and ADI_EXIT_CRITICAL_REGION are 
    used. 
    
    @sa ADI_ENTER_CRITICAL_REGION()
    @sa ADI_EXIT_CRITICAL_REGION()
*/
#define ADI_INT_STATUS_ALLOC()     UINT threadx_interrupt_status;


    
/*! Macro to enter critical section. To use this macro, the
    interrupt status variable should be defined (ADI_INT_STATUS_ALLOC)
    in the same scope. 
    
    @sa ADI_INT_STATUS_ALLOC()
*/
#define ADI_ENTER_CRITICAL_REGION()                                                            \
        do {                                                                                   \
               threadx_interrupt_status = tx_interrupt_control(TX_INT_DISABLE);                \
        } while (0) 



/*! Macro to exit critical section.To use this macro, the
    interrupt status variable should be defined (ADI_INT_STATUS_ALLOC)
    in the same scope. 

    @sa  ADI_INT_STATUS_ALLOC()   
*/ 
#define ADI_EXIT_CRITICAL_REGION()                                                             \
        do {                                                                                   \
               threadx_interrupt_status = tx_interrupt_control(threadx_interrupt_status);      \
        } while (0)



/*! Code that uCOS requires to be run in the beginning of an interrupt handler. 
    @sa ISR_EPILOG()
*/

/* In Cortex-M processors ThreadX does not require a prolog or epilog and the handlers can be written in C */
#define ISR_PROLOG()           

/*! Code that uCOS requires to be run in the end of an interrupt handler. 
    @sa ISR_PROLOG()
*/
#define ISR_EPILOG()                                                                            \
                            

#endif /* __STDC__ */

#define PENDSV_HANDLER   __tx_PendSVHandler
#define SYSTICK_HANDLER  __tx_SysTickHandler
#define SVC_HANDLER      SVC_Handler
          

#endif /* ADI_RTOS_MAP_UCOS_III_H */
