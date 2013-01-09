/* /ADK/include/adk/avr/usb.h
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file usb.h
 * USB interface software implementation.
 */

#ifndef USB_H_
#define USB_H_

/* User application USB configuration file. Should be provided by the application.
 * It also should include <adk/avr/usb_config.h> file in the end of definitions.
 */
#include <usb_config.h>

#ifndef USB_CONFIG_INT_H_
#error "<adk/avr/usb_config.h> should be included by user usb_config.h file!"
#endif

#ifdef AVR_USB_DEBUG

/** Set debug token. The token is 4-bits integer which is output to the
 * configured debug port.
 */
#define AVR_USB_DBG_SET(__token) \
    (AVR_USB_DBGPORT_PORT = (AVR_USB_DBGPORT_PORT & 0xf0) | ((__token) & 0x0f))

#else /* AVR_USB_DEBUG */

#define AVR_USB_DBG_SET(__token)

#endif /* AVR_USB_DEBUG */

/** Prepare USB interface. */
void
AdkUsbSetup();

/** USB interrupt handler. The application should call this function when it
 * detects data line level change (data lines idle state is low D+ and high D-
 * level). It is up to application to set up interrupt and provide ISR which
 * will call this function. So any custom hardware mechanism can be used for
 * interrupt generation (e.g. separate circuit and port pin). Delay for ISR
 * prologue and function call is acceptable since this function will synchronize
 * with SYNC pattern which precedes each packet.
 * Keep in mind that all packet reception and response is handled inside this
 * function with interrupts globally disabled. This can consume a lot of CPU
 * time when large USB traffic is processed (even when destination is another
 * USB function).
 */
void
AdkUsbInterrupt();

/** This function should be called in the application main loop.
 * XXX functionality
 */
void
AdkUsbPoll();

#endif /* USB_H_ */
