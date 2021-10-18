/*! ****************************************************************************
 * @file:    adi_crypto_config.h
 * @brief:   Configuration options for Crypto driver.
 *           This is specific to the Crypto driver and will be included by the driver.
 *           It is not required for the application to include this header file.
 * -----------------------------------------------------------------------------
 * Copyright (c) 2014-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef __ADI_CRYPTO_CONFIG_H__
#define __ADI_CRYPTO_CONFIG_H__
#include <adi_global_config.h>

/** @addtogroup Crypto_Driver_Config Static Configuration
 *  @ingroup Crypto_Driver
 *  @{
 */

/************* Crypto Driver configurations ***************/

/*! Enable/Disable ECB Support\n
    1 - Enable ECB Support\n
    0 - Disable ECB Support\n
*/
#define ADI_CRYPTO_ENABLE_ECB_SUPPORT            (1)

/*! Enable/Disable CTR Support\n
    1 - Enable CTR Support\n
    0 - Disable CTR Support\n
*/
#define ADI_CRYPTO_ENABLE_CTR_SUPPORT            (1)

/*! Enable/Disable CBC Support\n
    1 - Enable CBC Support\n
    0 - Disable CBC Support\n
*/
#define ADI_CRYPTO_ENABLE_CBC_SUPPORT            (1)

/*! Enable/Disable CCM Support\n
    1 - Enable CCM Support\n
    0 - Disable CCM Support\n
*/
#define ADI_CRYPTO_ENABLE_CCM_SUPPORT            (1)

/*! Enable/Disable CMAC Support\n
    1 - Enable CMAC Support\n
    0 - Disable CMAC Support\n
*/
#define ADI_CRYPTO_ENABLE_CMAC_SUPPORT           (1)

/*! Enable/Disable HMAC Support\n
    1 - Enable HMAC Support\n
    0 - Disable HMAC Support\n
*/
#if defined (__ADUCM4x50__)
#define ADI_CRYPTO_ENABLE_HMAC_SUPPORT           (1)
#endif /*ADuCM4x50*/
/*! Enable/Disable SHA Support\n
    1 - Enable SHA Support\n
    0 - Disable SHA Support\n
*/
#define ADI_CRYPTO_ENABLE_SHA_SUPPORT            (1)


/*! Enable/Disable DMA Support\n
    1 - Enable DMA Support\n
    0 - Disable DMA Support
*/
#define ADI_CRYPTO_ENABLE_DMA_SUPPORT            (1)

/*! Enable/Disable DMA Transfer by default\n
    1 - Enable DMA \n
    0 - Disable DMA
*/
#define ADI_CRYPTO_ENABLE_DMA                    (1)

/*! SHA output format\n
    1 - Big-Endian \n
    0 - Little-Endian
*/
#define ADI_CRYPTO_SHA_OUTPUT_FORMAT             (1)


/*! Enable/Disable PKSTOR Support\n
    1 - Enable PKSTOR Support\n
    0 - Disable PKSTOR Support\n
*/
#define ADI_CRYPTO_ENABLE_PKSTOR_SUPPORT         (0)



/************** Macro validation *****************************/

#if ((ADI_CRYPTO_ENABLE_DMA_SUPPORT != 0) && (ADI_CRYPTO_ENABLE_DMA_SUPPORT != 1))
#error "ADI_CRYPTO_ENABLE_DMA_SUPPORT is invalid"
#endif

#if ((ADI_CRYPTO_ENABLE_DMA != 0) && (ADI_CRYPTO_ENABLE_DMA != 1))
#error "ADI_CRYPTO_ENABLE_DMA is invalid"
#endif

#if ((ADI_CRYPTO_ENABLE_DMA == 1) && (ADI_CRYPTO_ENABLE_DMA_SUPPORT == 0))
#error "DMA cannot be enabled if DMA support is disabled"
#endif

#if (!defined(__ADUCM4x50__) && (ADI_CRYPTO_ENABLE_PKSTOR_SUPPORT == 1))
#error "PKSTOR extensions only supported on ADuCM4x50 platform"
#endif

/*! @} */

#endif /* __ADI_CRYPTO_CONFIG_H__ */
