/*! ****************************************************************************
 * @file:    adi_i2c_data.c
 * @brief:   Data declaration for I2C Device Driver
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_I2C_DATA_C
#define ADI_I2C_DATA_C

 /*! \cond PRIVATE */

#include "adi_i2c_def.h"
#include "adi_i2c_config.h"

#if !defined(ADI_I2C_CFG_MCTL_BUSCLR)
/**
 * If the pragma message is printed, then ADI_I2C_CFG_MCTL_BUSCLR is not defined
 * in adi_i2c_config.h. This is probably caused by a local project configuration
 * file, adi_i2c_config.h, existing before a recent DFP pack was installed with
 * the newly added I2C configuration parameter.
 */
#pragma message("!!! I2C configuration parameters have been extended.")
#pragma message("!!! Consider defining ADI_I2C_CFG_MCTL_BUSCLR in the")
#pragma message("!!! adi_i2c_config.h used by your project.")
#pragma message("!!! (Macro ADI_I2C_CFG_MCTL_BUSCLR is defaulted to 0.)")
/*! Master control register busclear enable.\n
    1-Enable busclear bit for generating extra nine clock cycles
	  when I2C bus hangs
    0-Disable busclear bit. */ 
#define ADI_I2C_CFG_MCTL_BUSCLR                 (0)
#endif

#if !defined(ADI_I2C_CFG_MCTL_STOPBUSCLR)
/**
 * If the pragma message is printed, then ADI_I2C_CFG_MCTL_STOPBUSCLR is not
 * defined in adi_i2c_config.h. This is probably caused by a local project
 * configuration file, adi_i2c_config.h, existing before a recent DFP pack
 * was installed with the newly added I2C configuration parameter.
 */
#pragma message("!!! I2C configuration parameters have been extended.")
#pragma message("!!! Consider defining ADI_I2C_CFG_MCTL_STOPBUSCLR in the")
#pragma message("!!! adi_i2c_config.h used by your project.")
#pragma message("!!! (Macro ADI_I2C_CFG_MCTL_STOPBUSCLR is defaulted to 0.)")
/*! Master control register stopbusclear enable.\n
    1-Enable stopbusclear bit for stopping the master to generate
	  extra clock cycles when SDA release
    0-Disable stopbusclear bit. */
#define ADI_I2C_CFG_MCTL_STOPBUSCLR             (0)
#endif


/* Stores the information about the specific device */
static ADI_I2C_DEVICE_INFO i2c_device_info [ADI_I2C_NUM_INSTANCES] =
{
    /* fixed instance data for the singular I2C0 controller */
    {
        I2C_MST_EVT_IRQn,              /* pio interrupt number         */
        (ADI_I2C_TypeDef *)pADI_I2C0,  /* i2c controller pointer       */
        NULL,                          /* pointer to user config data  */
        NULL                           /* i2c device handle (user mem) */
    },

    /* no other i2c instances at this time */
};

/* build I2C Application configuration array */
static ADI_I2C_CONFIG gConfigInfo[ADI_I2C_NUM_INSTANCES] =
{
    /* the one-and-only (so far) instance data for I2C, I2C0... */
    {
        /**** I2C_MCTL Master Control register *** */
        (
            /* note: Master IENMTX and IENMRX (transmit and receive interrupts)
               are managed dynamically */
            ( ADI_I2C_CFG_MCTL_STOPBUSCLR   << BITP_I2C_MCTL_STOPBUSCLR   ) | 
            ( ADI_I2C_CFG_MCTL_BUSCLR       << BITP_I2C_MCTL_BUSCLR       ) |
            ( ADI_I2C_CFG_MCTL_MXMITDEC     << BITP_I2C_MCTL_MXMITDEC     ) |
            ( ADI_I2C_CFG_MCTL_IENCMP       << BITP_I2C_MCTL_IENCMP       ) |
            ( ADI_I2C_CFG_MCTL_IENACK       << BITP_I2C_MCTL_IENACK       ) |
            ( ADI_I2C_CFG_MCTL_IENALOST     << BITP_I2C_MCTL_IENALOST     ) |
            ( ADI_I2C_CFG_MCTL_STRETCHSCL   << BITP_I2C_MCTL_STRETCHSCL   ) |
            ( ADI_I2C_CFG_MCTL_LOOPBACK     << BITP_I2C_MCTL_LOOPBACK     ) |
            ( ADI_I2C_CFG_MCTL_COMPLETE     << BITP_I2C_MCTL_COMPLETE     ) |
            ( ADI_I2C_CFG_MCTL_MASEN        << BITP_I2C_MCTL_MASEN        )
        ),

        /**** I2C_DIV Clock Divider register *** */
        (
            ( ADI_I2C_CFG_DIV_HIGH          << BITP_I2C_DIV_HIGH          ) |
            ( ADI_I2C_CFG_DIV_LOW           << BITP_I2C_DIV_LOW           )
        ),

        /**** I2C_SHCTL Shared Control register *** */
        (
            ( ADI_I2C_CFG_SHCTL_RST         << BITP_I2C_SHCTL_RST         )
        ),

        /**** I2C_TCTL Timing control register *** */
        (
            ( ADI_I2C_CFG_TCTL_FILTEROFF    << BITP_I2C_TCTL_FILTEROFF    ) |
            ( ADI_I2C_CFG_TCTL_THDATIN      << BITP_I2C_TCTL_THDATIN      )
        ),

        /**** I2C_ASTRETCH Master Clock Stretch register *** */
        (
            ( ADI_I2C_CFG_ASTRETCH_MST      << BITP_I2C_ASTRETCH_SCL_MST  )
        ),

        /**** Target Slave configuration value (not a register) *** */
        (
            ( ADI_I2C_CFG_SLAVE_ADDRESS                                   )
        ),
    }
};

/*! \endcond */


#endif /* ADI_I2C_DATA_C */
