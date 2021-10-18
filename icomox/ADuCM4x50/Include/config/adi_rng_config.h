/*! ****************************************************************************
 * @file:    adi_rng_config.h
 * @brief:   Configuration options for RNG driver.
 *           This is specific to the RNG driver and will be included by the driver.
 *           It is not required for the application to include this header file.
 * -----------------------------------------------------------------------------
 * Copyright (c) 2012-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_RNG_CONFIG_H__
#define ADI_RNG_CONFIG_H__
#include <adi_global_config.h>
/** @defgroup RNG_Driver_Cfg RNG Driver Configuration
 *  @ingroup RNG_Driver

 */

/*! \addtogroup  RNG_Driver_Cfg RNG Driver Configuration
 *  @{
 */

/************* RNG Driver configurations ***************/

/************* RNG controller configurations ***************/

/*! RNG Control Register, bit 3\n
    Enable only 8-bit generation\n
    0 -  Generate 32-bit random number\n
    1 -  Generate only 8-bit random number
*/
#define RNG0_CFG_ONLY_8_BIT                             1

/*! RNG Sample Length Register, bits [11:0]\n
    The register defines the number of samples to accumulate in the
    CRC register when generating a random number.\n

    Bits [11:0] contains the reload value of the sample counter

    */
#define RNG0_CFG_LENGTH_RELOAD                          256u

/*! RNG Sample Length Register, bits [15:12]\n
    The register defines the number of samples to accumulate in the
    CRC register when generating a random number. The number of values
    accumulated in the counter reload value is scaled by 2^prescaler.\n

    Bits [15:12] contains the prescaler for the sample counter

    */
#define RNG0_CFG_LENGTH_PRESCALER                       0u

/************** Macro validation *****************************/

#if ( RNG0_CFG_ONLY_8_BIT > 1 )
#error "Invalid configuration"
#endif

#if ( RNG0_CFG_LENGTH_RELOAD > 4095u )
#error "Invalid value for reload"
#endif

#if ( RNG0_CFG_LENGTH_PRESCALER > 10u )
#error "Invalid value for prescaler"
#endif

/*! @} */

#endif /* __ADI_RNG_CONFIG_H__ */
