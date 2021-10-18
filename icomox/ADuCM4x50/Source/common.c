/*! ****************************************************************************
 * @file:    common.c
 * @brief:   Common utilities for testing
 * -----------------------------------------------------------------------------
 * Copyright (c) 2010-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#include "common.h"

char aDebugString[150u];

#ifdef REDIRECT_OUTPUT_TO_UART

#include <drivers/uart/adi_uart.h>
#include <drivers/general/adi_drivers_general.h>

ADI_UART_HANDLE hDevOutput = NULL;

ADI_ALIGNED_PRAGMA(4)
uint8_t OutDeviceMem[ADI_UART_UNIDIR_MEMORY_SIZE] ADI_ALIGNED_ATTRIBUTE(4);

#define UART0_TX_PORTP0_MUX (1u<<20)
#define UART0_RX_PORTP0_MUX (1u<<22)

#endif /* REDIRECT_OUTPUT_TO_UART */

/**
 * Test Initialization
 *
 * @brief  Test initialization
 *
 */
void common_Init(void)
{     
#ifdef REDIRECT_OUTPUT_TO_UART
    /* Set the pinmux for the UART */
    *pREG_GPIO0_CFG |= UART0_TX_PORTP0_MUX | UART0_RX_PORTP0_MUX;

    /* Open the UART device, data transfer is bidirectional with NORMAL mode by default */
    adi_uart_Open(0u, ADI_UART_DIR_TRANSMIT, OutDeviceMem, sizeof OutDeviceMem, &hDevOutput);
#endif
}


/**
 * Passing Result
 *
 * @brief  Report a passing test result
 *
 */
void common_Pass(void)
{
    char pass[] = "All done!\n\r";
    
#ifdef REDIRECT_OUTPUT_TO_UART
    uint32_t pHwError;
    /* Ignore return codes since there's nothing we can do if it fails */
    adi_uart_Write(hDevOutput, pass, strlen(pass), false, &pHwError);
#else
    printf(pass);
#endif
}


/**
 * Failing Result
 *
 * @brief  Report a failing test result
 *
 * @param  [in] FailureReason : String identifying the failure
 *
 */
void common_Fail(char *FailureReason)
{
    char fail[] = "Failed: ";
    char term[] = "\n\r";

#ifdef REDIRECT_OUTPUT_TO_UART
    uint32_t pHwError;
    /* Ignore return codes since there's nothing we can do if it fails */
    adi_uart_Write(hDevOutput, fail, strlen(fail), false, &pHwError);
    adi_uart_Write(hDevOutput, FailureReason, strlen(FailureReason), false, &pHwError);
    adi_uart_Write(hDevOutput, term, strlen(term), false, &pHwError);
#else
    printf(fail);
    printf(FailureReason);
    printf(term);
#endif
 }


/**
 * Info
 *
 * @brief  Report a failing test result
 *
 * @param  [in] InfoString : String with test information
 *
 */
void common_Perf(char *InfoString)
{
    char term[] = "\n\r";
    
#ifdef REDIRECT_OUTPUT_TO_UART
    uint32_t pHwError;
    /* Ignore return codes since there's nothing we can do if it fails */
    adi_uart_Write(hDevOutput, InfoString, strlen(InfoString), false, &pHwError);
    adi_uart_Write(hDevOutput, term, strlen(term), false, &pHwError);
#else
    printf(InfoString);
    printf(term);
#endif
}
