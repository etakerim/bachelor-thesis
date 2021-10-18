/*! ****************************************************************************
 * @file:    adi_rtos_map.h
 * @brief:   RTOS API mapping file.
 *           This is the main RTOS mapping header file which will include other
 *           RTOS mapping files based on the RTOS selection.
 * 
 * @details: The purpose of RTOS mapping file is for mapping the abstracted
 *           RTOS macros to the RTOS API calls based on the chosen RTOS.
 * 
 *           NOTE: This file is intended to be used by only the drivers. Not at
 *                 the application level.
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2020 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_RTOS_MAP_H
#define ADI_RTOS_MAP_H

#include <adi_global_config.h>

#if   (ADI_CFG_RTOS == ADI_CFG_RTOS_MICRIUM_III)
    
#include "rtos_map/adi_rtos_map_ucos_iii.h"

#elif   (ADI_CFG_RTOS == ADI_CFG_RTOS_MICRIUM_II)
    
#include "rtos_map/adi_rtos_map_ucos_ii.h"

#elif (ADI_CFG_RTOS == ADI_CFG_RTOS_FREERTOS)
    
#include "rtos_map/adi_rtos_map_freertos.h"

#elif (ADI_CFG_RTOS == ADI_CFG_RTOS_THREADX)
    
#include "rtos_map/adi_rtos_map_threadx.h"

#else 
    
#include "rtos_map/adi_rtos_map_noos.h"

#endif /* ADI_CFG_RTOS_MICRIUM_III */ 

#endif /* ADI_RTOS_MAP_H */
