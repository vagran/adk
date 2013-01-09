/* /ADK/src/libavr/usb/usb_tranceiver.S
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file usb_tranceiver.S
 * Time-critical low-level code for USB interface implementation.
 */

#include <avr/io.h>
#include <adk/defs.h>
#include <usb_config.h>

/** Mask for D+ and D- lines. */
#define DATA_LINES_MASK     (_BV(AVR_USB_DPLUS_PIN) | _BV(AVR_USB_DMINUS_PIN))
/** Masked value for J state. */
#define DATA_LINES_J        _BV(AVR_USB_DPLUS_PIN)
/** Masked value for K state. */
#define DATA_LINES_K        _BV(AVR_USB_DMINUS_PIN)
/** Masked value for idle state. */
#define DATA_LINES_IDLE     DATA_LINES_K
/** Masked value for SE0 (single-ended zero) state. */
#define DATA_LINES_SE0      0
/** Masked value for SE1 (single-ended one) state. */
#define DATA_LINES_SE1      (_BV(AVR_USB_DPLUS_PIN) | _BV(AVR_USB_DMINUS_PIN))

/* All the physical communications over USB bus are handled in this function.
 * The code is intented to be used with 20MHz crystal. So the timing is 13.33
 * CPU cycles per bit.
 */
.global AdkUsbInterrupt
.type   AdkUsbInterrupt, @function
AdkUsbInterrupt:

    /* First thing which must be done is synchronizing with SYNC pattern. For
     * low-speed function it must be KJKJKJKK. It is not known when this
     * function is entered relatively to the SYNC pattern position because of
     * interrupt handler calling delay and ISR prologue. It is only must be
     * guaranteed that it is not later than last the J token which is plenty of
     * time (66 cycles). Only D+ line is checked assuming that diferential
     * signal is applied.
     */

    sbis AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN

    //XXX
    ret
