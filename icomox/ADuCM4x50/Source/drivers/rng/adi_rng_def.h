/*! ****************************************************************************
 * @file:    adi_rng_def.h
 * @brief:   Random Number Generator Driver private data structures
 * -----------------------------------------------------------------------------
 * Copyright (c) 2012-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_RNG_DEF_H
#define ADI_RNG_DEF_H

 /*! \cond PRIVATE */


/*! RNG device internal instance data structure */
typedef struct __ADI_RNG_DEV_DATA_TYPE
{
    IRQn_Type                      IRQn;      /*!< RNG interrupt number       */
    ADI_CALLBACK                   CBFunc;    /*!< Callback function          */
    void                          *pCBParam;  /*!< Callback parameter         */
} ADI_RNG_DEV_DATA_TYPE;

/*! RNG device internal data structure */
typedef struct __ADI_RNG_DEV_TYPE
{
    volatile ADI_RNG_TypeDef      *pRNG;      /*!< MMR address for this RNG   */
    ADI_RNG_DEV_DATA_TYPE         *pData;     /*!< Pointer to instance data   */
} ADI_RNG_DEV_TYPE;


/*! \endcond */
#endif /* ADI_RNG_DEF_H */
