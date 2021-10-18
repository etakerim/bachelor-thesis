/*! ****************************************************************************
 * @file:    adi_beep_def.h
 * @brief:   BEEP Device Driver definition
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef _ADI_BEEP_DEF_H_
#define _ADI_BEEP_DEF_H_

/*! \cond PRIVATE */
#include <drivers/beep/adi_beep.h>

/*!
 *****************************************************************************
 * An interrupt mask covering all Beeper interrupts.
 *****************************************************************************/
#define ADI_BEEP_ALL_INTERRUPTS ( BITM_BEEP_CFG_SEQATENDIRQ         \
                                | BITM_BEEP_CFG_SEQNEARENDIRQ    \
                                | BITM_BEEP_CFG_BENDIRQ       \
                                | BITM_BEEP_CFG_BSTARTIRQ     \
                                | BITM_BEEP_CFG_AENDIRQ       \
                                | BITM_BEEP_CFG_ASTARTIRQ)

#define ADI_BEEP_TONE_DISABLE               (BITM_BEEP_TONEA_DIS)       /*!<  Beeper tone disable bit                          */

#define ADI_BEEP_TONE_FREQ_BITPOS           (BITP_BEEP_TONEA_FREQ)      /*!<  Beeper tone frequency bitfield position          */
#define ADI_BEEP_TONE_DUR_BITPOS            (BITP_BEEP_TONEA_DUR)       /*!<  Beeper tone duration bitfield position           */

#define ADI_BEEP_TONE_FREQ_MASK             (BITM_BEEP_TONEA_FREQ)      /*!<  Beeper tone frequency bitfield mask              */
#define ADI_BEEP_TONE_DUR_MASK              (BITM_BEEP_TONEA_DUR)       /*!<  Beeper tone duration bitfield mask               */

/*!
 *****************************************************************************
 * ADI_BEEP_STATE
 *
 * BEEP driver state.  Used for internal tracking of the BEEP device initialization
 * progress during the adi_beep_Open().  Also used to insure the BEEP device has been
 * properly initialized as a prerequisite to using the balance of the BEEP API.
 *
 *****************************************************************************/
typedef uint8_t ADI_BEEP_STATE;
#define ADI_BEEP_STATE_UNINITIALIZED  0u               /*!< BEEP is not initialized.              */
#define ADI_BEEP_STATE_INITIALIZED    (1u << 1u)       /*!< BEEP is initialized.                  */
#define ADI_BEEP_STATE_PLAYING        (1u << 2u)       /*!< BEEP is currently playing.            */
#define ADI_BEEP_STATE_BLOCKED        (1u << 3u)       /*!< BEEP has blocked, waiting completion. */

/*!
 * \struct ADI_BEEP_DEV_DATA
 * Beeper device internal instance data structure.
 */
typedef struct _ADI_BEEP_DEV_DATA
{
    volatile ADI_BEEP_STATE   state;          /*!< Device state                */
    ADI_CALLBACK              cbFunc;         /*!< Callback function           */
    void                      *cbParam;       /*!< Callback parameter          */
#if ADI_BEEP_INCLUDE_PLAY_SEQUENCE == 1
    ADI_BEEP_NOTE             (*pSeqArray)[];   /*!< Pointer to a user-allocated array of notes. */
    volatile uint8_t            nSeqIndex;      /*!< Index for incrementing sequence */
    uint8_t                     nSeqMax;        /*!< Size of the sequence */
#endif
    SEM_VAR_DECLR      
} ADI_BEEP_DEV_DATA;


/*! \struct ADI_BEEP_DRIVER_STRUCT
 *  BEEP Device Structure
 */
typedef struct _ADI_BEEP_DRIVER_STRUCT
{
    ADI_BEEP_TypeDef   *pReg;          /*!< Pointer to register base           */
    ADI_BEEP_DEV_DATA  *pData;         /*!< Pointer to device data structure   */  
} ADI_BEEP_DRIVER_STRUCT;

/*! \struct ADI_BEEP_STATIC_INIT
 *  conditionally create static initialization data based on adi_beep_config.h settings
 */
typedef struct {
	uint16_t BEEP_CFG;   /*!< Beeper configuration register */
	uint16_t BEEP_STAT;  /*!< Beeper status register */
	uint16_t BEEP_TONEA; /*!< Beeper ToneA register */
	uint16_t BEEP_TONEB; /*!< Beeper ToneB register */
} ADI_BEEP_STATIC_INIT;

/* alias for the actual device structure */
typedef ADI_BEEP_DRIVER_STRUCT      ADI_BEEP_DRIVER;

/*! \endcond */

#endif /* _ADI_BEEP_DEF_H_ */
