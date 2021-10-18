/*! ****************************************************************************
 * @file:    adi_rtos_map_ucos_iii.h
 * @brief:   uCOS-III RTOS API mapping file.
 *           This file maps the RTOS macros to uCOS-III APIs
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_RTOS_MAP_UCOS_III_H
#define ADI_RTOS_MAP_UCOS_III_H

/* If building a c file */
#if defined(__STDC__)

#include <cpu.h>
#include <os.h>
#include <stdint.h>
#include <stddef.h>

/*! Macro that declares the semaphore type that the drivers use. 
    The macro should be used within the device data structure.
    It should not be used to declare the semaphore as a global variable. */
#define SEM_VAR_DECLR                                                                           \
                    OS_SEM Semaphore;                                  

/*! Memory required for semaphore in terms bytes. This size is used to compute 
    the total memory required for the operation of the driver. uCOS-III requires
    semaphore memory to be passed by application. But there is no memory required
    to store the handle. For every semaphore related call the same memory pointer 
    that was used during create will be passed. */
#define ADI_SEM_SIZE        (sizeof(OS_SEM))
                                        
/*! Macro that creates a semaphore and returns the error specified in case of failure. DEV is the handle to the device driver structure that contains the semaphore/semaphore handle. */
#define SEM_CREATE(DEV, name, error)                                                            \
                   do {                                                                         \
                         OS_ERR os_error;                                                       \
                         OSSemCreate(&((DEV)->Semaphore), name ,0u, &os_error);                 \
                         if(OS_ERR_NONE != os_error) {return((error));}                         \
                      } while (0)

/*! Macro that deletes a semaphore and returns the error specified in case of failure. DEV is the handle to the device driver structure that contains the semaphore/semaphore handle. */
#if (OS_CFG_SEM_DEL_EN == DEF_ENABLED)
#define SEM_DELETE(DEV, error)                                                                  \
                   do {                                                                         \
                         OS_ERR os_error;                                                       \
                         OSSemDel( &((DEV)->Semaphore), OS_OPT_DEL_NO_PEND, &os_error );        \
                         if(OS_ERR_NONE != os_error) {return((error));}                         \
                      } while (0)
#else
#define SEM_DELETE(DEV, error) do { } while(0)
#endif

/*! Macro that blocks indefinitely on a semaphore and returns error in case of failure. DEV is the handle to the device driver structure that contains the semaphore handle.*/
#define SEM_PEND(DEV, error)                                                                    \
        do {                                                                                    \
              OS_ERR  os_error;                                                                 \
              OSSemPend    (&((DEV)->Semaphore), 0u, OS_OPT_PEND_BLOCKING , NULL, &os_error);   \
              if(OS_ERR_NONE != os_error) {return((error));}                                    \
           } while (0)
               
/*! Macro that posts a semaphore. DEV is the handle to the device driver structure that contains the semaphore handle. */
#define SEM_POST(DEV)                                                                           \
        do {                                                                                    \
          OS_ERR  os_error;                                                                     \
          OSSemPost(&((DEV)->Semaphore), OS_OPT_POST_1, &os_error);                             \
        } while (0)
          

/*! Defines a local variable where interrupt status register value is stored. 
    This macro should be used within a function in which critical section
    macros ADI_ENTER_CRITICAL_REGION and ADI_EXIT_CRITICAL_REGION are 
    used. 
    
    @sa ADI_ENTER_CRITICAL_REGION()
    @sa ADI_EXIT_CRITICAL_REGION()
*/
#define ADI_INT_STATUS_ALLOC()     CPU_SR_ALLOC()
    
/*! Macro to enter critical section. To use this macro, the
    interrupt status variable should be defined (ADI_INT_STATUS_ALLOC)
    in the same scope. 
    
    @sa ADI_INT_STATUS_ALLOC()
*/
#define ADI_ENTER_CRITICAL_REGION()  CPU_CRITICAL_ENTER()

/*! Macro to exit critical section.To use this macro, the
    interrupt status variable should be defined (ADI_INT_STATUS_ALLOC)
    in the same scope. 

    @sa  ADI_INT_STATUS_ALLOC()   
*/ 
#define ADI_EXIT_CRITICAL_REGION() CPU_CRITICAL_EXIT()



#if defined(ADI_CYCLECOUNT_ENABLED) && (ADI_CYCLECOUNT_ENABLED == 1)
#define ADI_RTOS_UCOS_III_CYCLECOUNT_START adi_cyclecount_start();
#define ADI_RTOS_UCOS_III_CYCLECOUNT_STOP  adi_cyclecount_stop();
#else
#define ADI_RTOS_UCOS_III_CYCLECOUNT_START 
#define ADI_RTOS_UCOS_III_CYCLECOUNT_STOP  
#endif

/*! Code that uCOS requires to be run in the beginning of an interrupt handler. 
    @sa ISR_EPILOG()
*/
#define ISR_PROLOG()                                                                            \
                            do {                                                                \
                                    CPU_SR_ALLOC();                                             \
                                    CPU_CRITICAL_ENTER();                                       \
                                    OSIntEnter();                                               \
                                    CPU_CRITICAL_EXIT();                                        \
                                    ADI_RTOS_UCOS_III_CYCLECOUNT_START                          \
                               } while (0);

/*! Code that uCOS requires to be run in the end of an interrupt handler. 
    @sa ISR_PROLOG()
*/
#define ISR_EPILOG()                                                                            \
                            do {                                                                \
                                   ADI_RTOS_UCOS_III_CYCLECOUNT_STOP                            \
                                   OSIntExit();                                                 \
                            } while (0);                                                        \

#endif /* __STDC__ */

#define PENDSV_HANDLER   OS_CPU_PendSVHandler
#define SYSTICK_HANDLER  OS_CPU_SysTickHandler
#define SVC_HANDLER      SVC_Handler
          

#endif /* ADI_RTOS_MAP_UCOS_III_H */
