/*! ****************************************************************************
 * @file:    retarget_uart_config.h
 * @brief:   I/O redirection support over UART, via SSL/DD 
 * @details: This file is intended for use with the ARM:Compiler:IO:*:User
 *           components, which set up redirection of stdout and stderr.
 * -----------------------------------------------------------------------------
 * Copyright(c) 2017-2020 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef RETARGET_UART_CONFIG_H
#define RETARGET_UART_CONFIG_H

// --- <<< Use Configuration Wizard in Context Menu >>> ---

// <h>UART Configuration for STDOUT and STDERR

// <q>Configure Pinmuxing for UART.
// <i>Enable pinmux configuration for UART on first output.
#define ADI_UART_SETUP_PINMUX 1

// <q>Raise Breakpoint on exit()
// <i>Cause a breakpoint event in exit() rather than looping forever.
#define ADI_UART_EXIT_BREAKPOINT 1


// </h>

#endif /* RETARGET_UART_CONFIG_H */
