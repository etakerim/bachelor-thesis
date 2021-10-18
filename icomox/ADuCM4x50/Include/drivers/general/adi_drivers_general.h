/*! ****************************************************************************
 * @file:    adi_drivers_general.h
 * @brief:   Macros and types used in multiple drivers
 * -----------------------------------------------------------------------------
 * Copyright (c) 2010-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_DRIVERS_GENERAL__H
#define ADI_DRIVERS_GENERAL__H


/* Macros related to alignment in the different toolchains supported */

/* 
 * These macros are designed to be used as follows:
 * ADI_ALIGNED_PRAGMA(<required_alignment>)
 * <variable_declaration> ADI_ALIGNED_ATTRIBUTE(<required_alignment>)
 */

#if defined ( __ICCARM__ )
/*
* IAR MISRA C 2004 error suppressions.
*
*
* Pm120 (rule 19.10): In the definition of a function-like macro each parameter
*                     shall be enclosed in parenthesis. 
*                     This is not possible in attributes and pragmas
* Pm154 (rule 19.13): The # and ## preprocessor operators shall not be used.
*                     We need to do this to abstract the macros for the 
*                     different toolchains
*/
#pragma diag_suppress=Pm120,Pm154
#endif

#define PRAGMA(x) _Pragma(#x)
#define ATTRIBUTE(x) __attribute__((x))

#if defined (__GNUC__)
  /* Gcc uses attributes */
  #define ADI_ALIGNED_PRAGMA(num)
  #define ADI_ALIGNED_ATTRIBUTE(num) ATTRIBUTE(aligned(num))
  #define ADI_UNUSED_ATTRIBUTE ATTRIBUTE(unused)
#elif defined ( __ICCARM__ )
  /* IAR uses a pragma */
  #define ADI_ALIGNED_ATTRIBUTE(num)
  #define ADI_ALIGNED_PRAGMA(num) PRAGMA(data_alignment=num) 
  #define ADI_UNUSED_ATTRIBUTE
#elif defined (__CC_ARM)
  /* Keil uses a decorator which is placed in the same position as pragmas */
  #define ADI_ALIGNED_ATTRIBUTE(num)
  #define ADI_ALIGNED_PRAGMA(num) __align(##num)
  #define ADI_UNUSED_ATTRIBUTE ATTRIBUTE(unused)
#else
#error "Toolchain not supported"
#endif


#if defined ( __ICCARM__ )
#pragma diag_default=Pm120,Pm154
#endif
#endif /* ADI_DRIVERS_GENERAL__H */
