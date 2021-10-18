/*! ****************************************************************************
 * @file:    adi_xint_def.h
 * @brief:   External Interrupt Driver definition
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_XINT_DEF_H
#define ADI_XINT_DEF_H
/*! \cond PRIVATE */

/* General macros */
#define ADI_XINT_CFG_BITS       (4u)       /*!< number of bits for each external interrupt configuration */

/*! Structure to hold callback function and parameter */
typedef struct _ADI_XINT_CALLBACK_INFO
{
    ADI_CALLBACK         pfCallback;        /*!< Callback function pointer */
    void                 *pCBParam;         /*!< Callback parameter */
} ADI_XINT_CALLBACK_INFO;


/*! \endcond */
#endif /* ADI_XINT_DEF_H */
