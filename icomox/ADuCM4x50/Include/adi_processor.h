/*! ****************************************************************************
 * @file:    adi_processor.h
 * @brief:   Include appropriate CMSIS device header.
 * -----------------------------------------------------------------------------
 * Copyright (c) 2010-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef __ADI_PROCESSOR_H__
#define __ADI_PROCESSOR_H__

#if defined(__ADUCM3029__)
#include <ADuCM3029.h>
#ifndef __ADUCM302x__
#define __ADUCM302x__
#endif
#endif

#if defined(__ADUCM3029_1__)
#include <ADuCM3029_1.h>
#ifndef __ADUCM302x__
#define __ADUCM302x__
#endif
#endif

#if defined(__ADUCM3029_2__)
#include <ADuCM3029_2.h>
#ifndef __ADUCM302x__
#define __ADUCM302x__
#endif
#endif

#if defined(__ADUCM3027__)
#include <ADuCM3027.h>
#ifndef __ADUCM302x__
#define __ADUCM302x__
#endif
#endif

/* Include CMSIS device header for selected target processor. */

#if defined(__ADUCM4050__)
#include <ADuCM4050.h>
#ifndef __ADUCM4x50__
#define __ADUCM4x50__
#endif
#endif

#endif /* __ADI_PROCESSOR_H__ */
