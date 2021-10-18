/*! ****************************************************************************
 * @file:    adi_version.h
 * @brief:   Version macros for ADI ADuCMxxx Device Series.
 * -----------------------------------------------------------------------------
 * Copyright(c) 2010-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef __ADI_VERSION_H__
#define __ADI_VERSION_H__

/* use a 32-bit versioning scheme that supports numerical compares */
#define ADI_VERSION_MAJOR     1u     /* must be <= 255 */
#define ADI_VERSION_MINOR     0u     /* must be <= 255 */
#define ADI_VERSION_BUILD     0u     /* must be <= 255 */
#define ADI_VERSION_PATCH     0u     /* must be <= 255 */

#define ADI_CONSTRUCT_VERSION(a,b,c,d)  (((a) << 24u) | ((b) << 16u) | ((c) << 8u) | (d))

/* known versions */
#define ADI_VERSION_1_0_0_0   ADI_CONSTRUCT_VERSION(1u,0u,0u,0u)

/* test current version against known predefines (see SystemInit() example in system.c) */
#define ADI_VERSION_CURRENT   ADI_CONSTRUCT_VERSION(ADI_VERSION_MAJOR, ADI_VERSION_MINOR, ADI_VERSION_BUILD, ADI_VERSION_PATCH)

#endif /* __ADI_VERSION_H__ */
