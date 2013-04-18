/*****************************************************************************
 *
 * \file
 *
 * \brief DACIFB header for AVR32 UC3.
 *
 * This file defines a useful set of functions for DACIFB on AVR32 devices.
 *
 * Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 *    AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 *****************************************************************************/


#ifndef _DACIFB_H_
#define _DACIFB_H_

/**
 * \defgroup group_avr32_drivers_dacifb ANALOG - DACIFB - DAC Interface B
 *
 * Driver for the Digital to Analog Converter (DAC) Interface B. It provides functions for configuring and using the DAC and its channels.
 *
 * \{
 */

#include <avr32/io.h>
#include "compiler.h"

#if UC3C
#define AVR32_FLASHC_CALIBRATION_FOURTH_WORD_ADDRESS    0x8080020C
#define AVR32_FLASHC_CALIBRATION_FIVETH_WORD_ADDRESS    0x80800214
#endif

//! Local Definition for Channel Selection used by the DAC
//!
#define  DACIFB_CHANNEL_SELECTION_NONE              0x0
#define  DACIFB_CHANNEL_SELECTION_A                 0x1
#define  DACIFB_CHANNEL_SELECTION_B                 0x2
#define  DACIFB_CHANNEL_SELECTION_AB                0x3

//! Local Definition for Reference used by the DAC
//!
#define  DACIFB_REFERENCE_EXT                       0x0
#define  DACIFB_REFERENCE_VDDANA                    0x1

//! Local Definition for Trigger Mode used by the DAC
//!
#define  DACIFB_TRIGGER_MODE_MANUAL                 0x0
#define  DACIFB_TRIGGER_MODE_TIMER                  0x1
#define  DACIFB_TRIGGER_MODE_EVENT                  0x2

//! Status
// ----------
//! This constant is used as return value for "dacifb_configure_core" and "adcifa_configure_sequencer" functions.
#define  DACIFB_CONFIGURATION_REFUSED                0x0
// ----------
//! This constant is used as return value for "adcifa_configure_core" and "adcifa_configure_sequencer" functions.
#define  DACIFB_CONFIGURATION_ACCEPTED               0x1
// ----------
//! This constant is used as return value for "adcifa_get_values_seq" function.
#define  DACIFB_STATUS_COMPLETED                     0x2
// ----------
//! This constant is used as return value for "adcifa_get_values_seq" function.
#define  DACIFB_STATUS_NOT_COMPLETED                 0x3

//! Local Definition for CHI Min Value of 2us
//!
#define  DACIFB_CHI_MIN_VALUE                        500000

//! Local Definition for CHRx Min Value of 25us
//!
#define  DACIFB_CHRx_MIN_VALUE                        40000

//! Local Definition for CHRx Max Value of 35us
//!
#define  DACIFB_CHRx_MAX_VALUE                        33333

//! Parameters for the DACIFB.
typedef struct
{
  //! Reference for DAC Conversion
  bool reference;

  //! Channel Selection
  U8  channel_selection;

  //! Gain Calibration Value
  U16 gain_calibration_value;

  //! Offset Calibration Value
  U16 offset_calibration_value;

  //! Low Power Mode
  bool low_power;

  //! Dual Mode
  bool dual;

  //! Prescaler Clock in Hertz (should be > 500000Hz)
  U32 prescaler_clock_hz;

} dacifb_opt_t;

//! Parameters for the configuration of the channel.
typedef struct
{
  //! Auto Refresh Mode.
  bool auto_refresh_mode;

  //! Trigger Mode: Manual/Timer/Event
  U8 trigger_mode;

  //! Left or Right Adjustment.
  bool left_adjustment;

  //! Data Shift Value.
  U8 data_shift;

  //! Data Round.
  bool data_round_enable;

} dacifb_channel_opt_t;

/*! \brief Get DACIFB Calibration Data. Mandatory to call if factory calibration data are wanted to be used.
 * If not called, Calibration Data should be set by the application.
 * \param *dacifb       Base address of the DACIFB
 * \param *p_dacifb_opt Structure for the DACIFB core configuration
 * \param instance      DACIFB core instance 0 for DACIFB0 or 1 for DACIFB1
 */
extern void dacifb_get_calibration_data(  volatile avr32_dacifb_t * dacifb,
                                          dacifb_opt_t * p_dacifb_opt,
                                          U8 instance);

/*! \brief Configure DACIFB. Mandatory to call.
 * If not called, DACIFB channels will have side effects
 *
 * \param *dacifb      Base address of the DACIFB
 * \param *p_dacifb_opt Structure for the DACIFB core configuration
 * \param pb_hz        Periphal Bus frequency
 * \return U8          DACIFB_CONFIGURATION_REFUSED or DACIFB_CONFIGURATION_ACCEPTED
 */
extern U8 dacifb_configure(volatile avr32_dacifb_t * dacifb,
                           dacifb_opt_t * p_dacifb_opt,
                           U32 pb_hz);

/*! \brief Configure DACIFB specific channel.
 *         - Adjustment, Refresh_time, Trigger Mode
 * \param  *dacifb                Base address of the ADCIFA
 * \param  channel                DACIFB_CHANNEL_SELECTION_NONE / DACIFB_CHANNEL_SELECTION_A / DACIFB_CHANNEL_SELECTION_B / DACIFB_CHANNEL_SELECTION_AB
 * \param  p_dacifb_channel_opt   Structure for the sequencer configuration
 * \param  prescaler_clock_hz     Prescaler Clock in Hertz (should be > 500000Hz)
 * \return U8                     DACIFB_CONFIGURATION_REFUSED or DACIFB_CONFIGURATION_ACCEPTED
 */
extern U8 dacifb_configure_channel(volatile avr32_dacifb_t * dacifb,
                                   U8 channel,
                                   dacifb_channel_opt_t * p_dacifb_channel_opt,
                                   U32 prescaler_clock_hz);

/*! \brief Start analog to digital conversion
 * \param *dacifb Base address of the DACIFB
 * \param  channel                DACIFB_CHANNEL_SELECTION_NONE / DACIFB_CHANNEL_SELECTION_A / DACIFB_CHANNEL_SELECTION_B / DACIFB_CHANNEL_SELECTION_AB
 * \param  cpu_hz                 CPU Clock frequency
*/
extern void dacifb_start_channel(volatile avr32_dacifb_t * dacifb,
                                 U8 channel,
                                 U32 cpu_hz);

/*! \brief Check channel conversion status
 *
 * \param *dacifb Base address of the DACIFB
 * \param  channel   channel to check (0 to 1)
 * \return bool      true if conversion not running
 *                   false if conversion running
 */
extern bool dacifb_check_eoc(volatile avr32_dacifb_t * dacifb,
                             U8 channel);
/*! \brief Set channel value
 *
 * \param *dacifb    Base address of the DACIFB
 * \param  channel   channel to handle (0 to 1)
 * \param  dual      Dual Mode Selection
 * \param  value     Value to be converted
 */
extern void dacifb_set_value(volatile avr32_dacifb_t * dacifb,
                            U8 channel,
                            bool dual,
                            U32 value);

/*! \brief Reload Timer for Automatic Trigger on DAC
 *  \param *dacifb  Base address of the DACIFB
 *  \param channel  DACIFB_CHANNEL_SELECTION_NONE / DACIFB_CHANNEL_SELECTION_A / DACIFB_CHANNEL_SELECTION_B / DACIFB_CHANNEL_SELECTION_AB
 * \param  timer_us Timer Value in Microsecondes
 * \param  prescaler_clock_hz     Prescaler Clock in Hertz (should be > 500000Hz)
*/
extern void dacifb_reload_timer(volatile avr32_dacifb_t * dacifb,
                                 U8 channel,
                                 U8 timer_us,
                                 U32 prescaler_clock_hz);

/**
 * \}
 */

#endif  // _DACIFB_H_
