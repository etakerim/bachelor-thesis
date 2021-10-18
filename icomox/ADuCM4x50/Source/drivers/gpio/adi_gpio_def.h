/*! ****************************************************************************
 * @file:    adi_gpio_def.h
 * @brief:   GPIO Device Driver definition
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_GPIO_DEF_H
#define ADI_GPIO_DEF_H
/*! \cond PRIVATE */


/*! local enum for callback table indexing */
typedef enum
{
    /* Group interrupts */
    ADI_GPIO_IRQ_GROUPA_INDEX     = (0x0),   /*!< GroupA interrupt index.   */
    ADI_GPIO_IRQ_GROUPB_INDEX     = (0x1),   /*!< GroupB interrupt index.   */

    ADI_GPIO_NUM_INTERRUPTS       = (0x2),   /*!< Number of GPIO interrupts */

} ADI_GPIO_IRQ_INDEX;


/*! Structure to hold callback function and parameter */
typedef struct _ADI_GPIO_CALLBACK_INFO
{
    ADI_CALLBACK         pfCallback;    /*!< Callback function pointer */
    void                 *pCBParam;      /*!< Callback parameter */
} ADI_GPIO_CALLBACK_INFO;

/*! Structure to hold callback function and parameter */
typedef struct _ADI_GPIO_DEV_DATA
{
    ADI_GPIO_CALLBACK_INFO  CallbackTable[ADI_GPIO_NUM_INTERRUPTS];  /*!< Callback Info for External interrupts */
} ADI_GPIO_DEV_DATA;

/*! \struct ADI_GPIO_DEVICE

    GPIO instance data

    This structure contains the "state" information for the
    instance of the device.  For GPIO there is only one
    of these objects.
*/
typedef struct _ADI_GPIO_DRIVER_STRUCT
{
	ADI_GPIO_TypeDef            *pReg[ADI_GPIO_NUM_PORTS];          /*!< GPIO Ports Register base          */
	ADI_GPIO_DEV_DATA           *pData;                             /*!< Pointer to device data            */
} ADI_GPIO_DRIVER_STRUCT;


/* alias for the actual device structure */
typedef ADI_GPIO_DRIVER_STRUCT      ADI_GPIO_DRIVER;

/*! \endcond */
#endif /* ADI_GPIO_DEF_H */
