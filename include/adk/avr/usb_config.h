/* /ADK/include/adk/avr/usb_config.h
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file usb_config.h
 * Default values for USB interface support configuration. Normally the
 * application should provide similar file where it should overwrite default
 * values as needed. This file must be included by the application usb_config.h
 * file.
 */

#ifndef USB_CONFIG_INT_H_
#define USB_CONFIG_INT_H_

/* Values which can be overwritten by the user. */

#ifndef AVR_USB_DATA_PORT
/** I/O port used for D+ and D- lines. Both lines must share the same port.
 * Hardware should be able to detect any one line level change. Data lines idle
 * state is low D+ and high D- level.
 */
#define AVR_USB_DATA_PORT   D
#endif /* AVR_USB_DATA_PORT */

#ifndef AVR_USB_DPLUS_PIN
/** Port pin for D+ data line. */
#define AVR_USB_DPLUS_PIN   3
#endif /* AVR_USB_DPLUS_PIN */

#ifndef AVR_USB_DMINUS_PIN
/** Port pin for D- data line. */
#define AVR_USB_DMINUS_PIN  2
#endif /* AVR_USB_DMINUS_PIN */

/* AVR_USB_DEBUG macro enables debug functionality. */
#ifdef AVR_USB_DEBUG

/** Port for output of debug token. Least significant four bits are used. */
#ifndef AVR_USB_DEBUG_PORT
#define AVR_USB_DEBUG_PORT  B
#endif /* AVR_USB_DEBUG_PORT */

#endif /* AVR_USB_DEBUG */

/* Internal definitions. */

#ifndef __ASSEMBLER__

/** PORTx register for data port. */
#define AVR_USB_DPORT_PORT  __CONCAT(PORT, AVR_USB_DATA_PORT)
/** DDRx register for data port. */
#define AVR_USB_DPORT_DDR   __CONCAT(DDR, AVR_USB_DATA_PORT)
/** PINx register for data port. */
#define AVR_USB_DPORT_PIN   __CONCAT(PIN, AVR_USB_DATA_PORT)

#ifdef AVR_USB_DEBUG

/** PORTx register for debug port. */
#define AVR_USB_DBGPORT_PORT  __CONCAT(PORT, AVR_USB_DEBUG_PORT)
/** DDRx register for debug port. */
#define AVR_USB_DBGPORT_DDR   __CONCAT(DDR, AVR_USB_DEBUG_PORT)

#endif /* AVR_USB_DEBUG */

#else /* __ASSEMBLER__ */

/* I/O space is shifted by 32 when using in assembler. */

/** PORTx register for data port. */
#define AVR_USB_DPORT_PORT  (__CONCAT(PORT, AVR_USB_DATA_PORT) - __SFR_OFFSET)
/** DDRx register for data port. */
#define AVR_USB_DPORT_DDR   (__CONCAT(DDR, AVR_USB_DATA_PORT) - __SFR_OFFSET)
/** PINx register for data port. */
#define AVR_USB_DPORT_PIN   (__CONCAT(PIN, AVR_USB_DATA_PORT) - __SFR_OFFSET)

#ifdef AVR_USB_DEBUG

/** PORTx register for debug port. */
#define AVR_USB_DBGPORT_PORT  (__CONCAT(PORT, AVR_USB_DEBUG_PORT) - __SFR_OFFSET)
/** DDRx register for debug port. */
#define AVR_USB_DBGPORT_DDR   (__CONCAT(DDR, AVR_USB_DEBUG_PORT) - __SFR_OFFSET)

#endif /* AVR_USB_DEBUG */

#endif /* __ASSEMBLER__ */

#endif /* USB_CONFIG_INT_H_ */
