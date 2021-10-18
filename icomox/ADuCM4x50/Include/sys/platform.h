/*! ****************************************************************************
 * @file:    platform.h
 * @brief:   Include appropriate architecture definitions.
 * -----------------------------------------------------------------------------
 * Copyright (c) 2010-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef __ADI_SYS_PLATFORM_H__
#define __ADI_SYS_PLATFORM_H__

/* Include the ADI cdef header for the selected target. */

#if defined(__ADUCM4050__)
#include <ADuCM4050_cdef.h>
#elif defined(__ADUCM3027__)
#include <ADuCM3027_cdef.h>
#elif defined(__ADUCM3029__)
#include <ADuCM3029_cdef.h>
#elif defined(__ADUCM3029_1__)
#include <ADuCM3029_1_cdef.h>
#elif defined(__ADUCM3029_2__)
#include <ADuCM3029_2_cdef.h>
#else
#error <sys/platform.h> not configured for this target.
#endif

#endif /* __ADI_SYS_PLATFORM_H__ */
