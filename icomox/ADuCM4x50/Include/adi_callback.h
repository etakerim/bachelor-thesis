/*! ****************************************************************************
 * @file:    adi_callback.h
 * @brief:   callback APIs.
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_CALLBACK_H
#define ADI_CALLBACK_H

#include <stdint.h>

/**
 * @brief Device Drivers Callback function definition
 */
typedef void (* ADI_CALLBACK) (  /*!< Callback function pointer */
    void      *pCBParam,         /*!< Client supplied callback param */
    uint32_t   Event,            /*!< Event ID specific to the Driver/Service */
    void      *pArg);            /*!< Pointer to the event specific argument */
    
#endif /* ADI_CALLBACK_H */
