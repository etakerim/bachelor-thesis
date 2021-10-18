/*! ****************************************************************************
 * @file:    common.h
 * @brief:   Common include file for all example 
 * -----------------------------------------------------------------------------
 * Copyright(c) 2010-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef COMMON_H
#define COMMON_H

#ifdef __ICCARM__
/*
* Pm106 (rule 20.9): the input/output library <stdio.h> shall not be used in
                     production code
*   The purpose of this header is to provide I/O facilities based on stdio.
*/
#pragma diag_suppress=Pm106
#endif /* __ICCARM__ */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>


#ifdef __ICCARM__
/*
* IAR MISRA C 2004 error suppressions.
*
* Pm011 (rule 6.3): The basic types of char, int, short, long, float shall not be used.
* Pm064 (rule 16.1): functions with variable number of arguments shall not be used.
*/
#pragma diag_suppress=Pm011,Pm064
#endif /* __ICCARM__ */


#ifdef __cplusplus
extern "C" {
#endif

/* Enable REDIRECT_OUTPUT_TO_UART to send the output to UART terminal. 
This requires the UART Driver(adi_uart.c) to be included in the project */
/* #define REDIRECT_OUTPUT_TO_UART */  

extern char aDebugString[150]; 

#ifdef __ICCARM__
/*
* Pm154 (rule 19.10): in the definition of a function-like macro, each instance
*                     of a parameter shall be enclosed in parentheses
*   The __VA_ARGS__ macro cannot be enclosed in parentheses.
*/
#pragma diag_suppress=Pm154
#endif /* __ICCARM__ */

#define DEBUG_MESSAGE(...) \
  do { \
    sprintf(aDebugString,__VA_ARGS__); \
    common_Perf(aDebugString); \
  } while(0)

#ifdef __ICCARM__
#pragma diag_default=Pm154
#endif /* __ICCARM__ */

#define DEBUG_RESULT(s,result,expected_value) \
  do { \
    if ((result) != (expected_value)) { \
      sprintf(aDebugString,"%s  %d", __FILE__,__LINE__); \
      common_Fail(aDebugString); \
      sprintf(aDebugString,"%s Error Code: 0x%08X\n\rFailed\n\r",(s),(result)); \
      common_Perf(aDebugString); \
      exit(0); \
    } \
  } while (0)

/********************************************************************************
* API function prototypes
*********************************************************************************/
void common_Init(void);
void common_Pass(void);
void common_Fail(char *FailureReason);
void common_Perf(char *InfoString);

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H */
