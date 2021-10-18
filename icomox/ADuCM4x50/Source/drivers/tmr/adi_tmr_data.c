/*! ****************************************************************************
 * @file    adi_tmr_data.c
 * @brief   GP and RGB timer static configuration data
 * -----------------------------------------------------------------------------
 * Copyright (c) 2016-2019 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * For license terms and conditions, please refer to the '*EZDFP Click Thru SLA'
 * file in the License subdirectory of this pack.
 *
 ******************************************************************************/
#ifndef ADI_TMR_DATA
#define ADI_TMR_DATA


#include <stdlib.h>
#include <adi_tmr_config.h>
#include <drivers/tmr/adi_tmr.h>

/* Macro mapping from ADuCM4x50 to ADuCM302x */
#if defined(__ADUCM302x__)
#define BITM_TMR_RGB_CTL_EN             BITM_TMR_CTL_EN
#define PWM0CTL                         PWMCTL
#define PWM0MATCH                       PWMMATCH
#define BITM_TMR_RGB_STAT_BUSY          BITM_TMR_STAT_BUSY  
#define BITM_TMR_RGB_CTL_EVTEN          BITM_TMR_CTL_EVTEN
#define BITM_TMR_RGB_CTL_RSTEN          BITM_TMR_CTL_RSTEN
#define BITP_TMR_RGB_CTL_RSTEN          BITP_TMR_CTL_RSTEN
#define BITP_TMR_RGB_CTL_EVTEN          BITP_TMR_CTL_EVTEN
#define BITP_TMR_RGB_CTL_PRE            BITP_TMR_CTL_PRE
#define BITP_TMR_RGB_CTL_CLK            BITP_TMR_CTL_CLK
#define BITP_TMR_RGB_CTL_MODE           BITP_TMR_CTL_MODE
#define BITP_TMR_RGB_CTL_UP             BITP_TMR_CTL_UP
#define BITP_TMR_RGB_CTL_RLD            BITP_TMR_CTL_RLD
#define BITP_TMR_RGB_CTL_SYNCBYP        BITP_TMR_CTL_SYNCBYP
#define BITP_TMR_RGB_PWM0CTL_IDLESTATE  BITP_TMR_PWMCTL_IDLESTATE
#define BITP_TMR_RGB_PWM0CTL_MATCH      BITP_TMR_PWMCTL_MATCH
#define BITM_TMR_RGB_CLRINT_TIMEOUT     BITM_TMR_CLRINT_TIMEOUT
#define BITM_TMR_RGB_STAT_PDOK          BITM_TMR_STAT_PDOK
#define BITM_TMR_RGB_STAT_TIMEOUT       BITM_TMR_STAT_TIMEOUT
#define BITM_TMR_RGB_STAT_CAPTURE       BITM_TMR_STAT_CAPTURE
#define BITM_TMR_RGB_CLRINT_EVTCAPT     BITM_TMR_CLRINT_EVTCAPT
#define BITM_TMR_RGB_CLRINT_TIMEOUT     BITM_TMR_CLRINT_TIMEOUT
#define BITM_TMR_RGB_CTL_RLD            BITM_TMR_CTL_RLD
#endif /*__ADUCM302x__*/

/* CTL register static configuration */
static uint16_t aTimerCtlConfig[] = 
{
    (TMR0_CFG_COUNT_UP              << BITP_TMR_RGB_CTL_UP)      |
    (TMR0_CFG_MODE                  << BITP_TMR_RGB_CTL_MODE)    |
    (TMR0_CFG_PRESCALE_FACTOR       << BITP_TMR_RGB_CTL_PRE)     |       
    (TMR0_CFG_CLOCK_SOURCE          << BITP_TMR_RGB_CTL_CLK)     |
    (TMR0_CFG_ENABLE_RELOADING      << BITP_TMR_RGB_CTL_RLD)     |
    (TMR0_CFG_ENABLE_SYNC_BYPASS    << BITP_TMR_RGB_CTL_SYNCBYP) |
    (TMR0_CFG_ENABLE_PRESCALE_RESET << BITP_TMR_RGB_CTL_RSTEN)   |
    (TMR0_CFG_ENABLE_EVENT_CAPTURE  << BITP_TMR_RGB_CTL_EVTEN),

    (TMR1_CFG_COUNT_UP              << BITP_TMR_RGB_CTL_UP)      |
    (TMR1_CFG_MODE                  << BITP_TMR_RGB_CTL_MODE)    |
    (TMR1_CFG_PRESCALE_FACTOR       << BITP_TMR_RGB_CTL_PRE)     |       
    (TMR1_CFG_CLOCK_SOURCE          << BITP_TMR_RGB_CTL_CLK)     |
    (TMR1_CFG_ENABLE_RELOADING      << BITP_TMR_RGB_CTL_RLD)     |
    (TMR1_CFG_ENABLE_SYNC_BYPASS    << BITP_TMR_RGB_CTL_SYNCBYP) |
    (TMR1_CFG_ENABLE_PRESCALE_RESET << BITP_TMR_RGB_CTL_RSTEN)   |
    (TMR1_CFG_ENABLE_EVENT_CAPTURE  << BITP_TMR_RGB_CTL_EVTEN),

    (TMR2_CFG_COUNT_UP              << BITP_TMR_RGB_CTL_UP)      |
    (TMR2_CFG_MODE                  << BITP_TMR_RGB_CTL_MODE)    |
    (TMR2_CFG_PRESCALE_FACTOR       << BITP_TMR_RGB_CTL_PRE)     |       
    (TMR2_CFG_CLOCK_SOURCE          << BITP_TMR_RGB_CTL_CLK)     |
    (TMR2_CFG_ENABLE_RELOADING      << BITP_TMR_RGB_CTL_RLD)     |
    (TMR2_CFG_ENABLE_SYNC_BYPASS    << BITP_TMR_RGB_CTL_SYNCBYP) |
    (TMR2_CFG_ENABLE_PRESCALE_RESET << BITP_TMR_RGB_CTL_RSTEN)   |
    (TMR2_CFG_ENABLE_EVENT_CAPTURE  << BITP_TMR_RGB_CTL_EVTEN),

#if defined(__ADUCM4x50__)
    (TMR3_CFG_COUNT_UP              << BITP_TMR_RGB_CTL_UP)      |
    (TMR3_CFG_MODE                  << BITP_TMR_RGB_CTL_MODE)    |
    (TMR3_CFG_PRESCALE_FACTOR       << BITP_TMR_RGB_CTL_PRE)     |       
    (TMR3_CFG_CLOCK_SOURCE          << BITP_TMR_RGB_CTL_CLK)     |
    (TMR3_CFG_ENABLE_RELOADING      << BITP_TMR_RGB_CTL_RLD)     |
    (TMR3_CFG_ENABLE_SYNC_BYPASS    << BITP_TMR_RGB_CTL_SYNCBYP) |
    (TMR3_CFG_ENABLE_PRESCALE_RESET << BITP_TMR_RGB_CTL_RSTEN)   |
    (TMR3_CFG_ENABLE_EVENT_CAPTURE  << BITP_TMR_RGB_CTL_EVTEN), 
#endif
};

/* LOAD register static configuration */
static uint16_t aTimerLoadConfig[] = 
{
    TMR0_CFG_LOAD_VALUE,
    TMR1_CFG_LOAD_VALUE,
    TMR2_CFG_LOAD_VALUE,
#if defined(__ADUCM4x50__)
    TMR3_CFG_LOAD_VALUE,
#endif
};

/* Asynchronous LOAD static configuraton */
static uint16_t aTimerALoadConfig[] = 
{
    TMR0_CFG_ASYNC_LOAD_VALUE,
    TMR1_CFG_ASYNC_LOAD_VALUE,
    TMR2_CFG_ASYNC_LOAD_VALUE,
#if defined(__ADUCM4x50__)
    TMR3_CFG_ASYNC_LOAD_VALUE,
#endif
};

/* EVENTSELECT static configuration */
#if defined(__ADUCM4x50__)
static uint16_t aTimerEventConfig[] =
{
    TMR0_CFG_EVENT_CAPTURE,
    TMR1_CFG_EVENT_CAPTURE,
    TMR2_CFG_EVENT_CAPTURE,
    TMR3_CFG_EVENT_CAPTURE,
};
#endif

/* PWM CTL static configuration */
static uint16_t aTimerPwmCtlConfig[] =
{
  (TMR0_CFG_PWM0_IDLE_STATE  << BITP_TMR_RGB_PWM0CTL_IDLESTATE) |
  (TMR0_CFG_ENABLE_PWM0_MATCH_MODE << BITP_TMR_RGB_PWM0CTL_MATCH),

  (TMR1_CFG_PWM0_IDLE_STATE  << BITP_TMR_RGB_PWM0CTL_IDLESTATE) |
  (TMR1_CFG_ENABLE_PWM0_MATCH_MODE << BITP_TMR_RGB_PWM0CTL_MATCH),

  (TMR2_CFG_PWM0_IDLE_STATE  << BITP_TMR_RGB_PWM0CTL_IDLESTATE) |
  (TMR2_CFG_ENABLE_PWM0_MATCH_MODE << BITP_TMR_RGB_PWM0CTL_MATCH),

#if defined(__ADUCM4x50__)
  (TMR3_CFG_PWM0_IDLE_STATE  << BITP_TMR_RGB_PWM0CTL_IDLESTATE) |
  (TMR3_CFG_ENABLE_PWM0_MATCH_MODE << BITP_TMR_RGB_PWM0CTL_MATCH),

  (TMR3_CFG_PWM1_IDLE_STATE  << BITP_TMR_RGB_PWM1CTL_IDLESTATE) |
  (TMR3_CFG_ENABLE_PWM1_MATCH_MODE << BITP_TMR_RGB_PWM1CTL_MATCH),

  (TMR3_CFG_PWM2_IDLE_STATE  << BITP_TMR_RGB_PWM2CTL_IDLESTATE) |
  (TMR3_CFG_ENABLE_PWM2_MATCH_MODE << BITP_TMR_RGB_PWM2CTL_MATCH),
#endif
};

/* PWM MATCH static configuration */
static uint16_t aTimerPwmMatchConfig[] = {
  TMR0_CFG_PWM0_MATCH_VALUE,
  TMR1_CFG_PWM0_MATCH_VALUE,
  TMR2_CFG_PWM0_MATCH_VALUE,
#if defined(__ADUCM4x50__)
  TMR3_CFG_PWM0_MATCH_VALUE,
  TMR3_CFG_PWM1_MATCH_VALUE,
  TMR3_CFG_PWM2_MATCH_VALUE
#endif
};


#endif /* ADI_TMR_DATA */
