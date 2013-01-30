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

#ifndef ADK_USB_DATA_PORT
/** I/O port used for D+ and D- lines. Both lines must share the same port.
 * Hardware should be able to detect any one line level change. Data lines idle
 * state is low D+ and high D- level.
 */
#define ADK_USB_DATA_PORT   D
#endif /* ADK_USB_DATA_PORT */

#ifndef ADK_USB_DPLUS_PIN
/** Port pin for D+ data line. */
#define ADK_USB_DPLUS_PIN   3
#endif /* ADK_USB_DPLUS_PIN */

#ifndef ADK_USB_DMINUS_PIN
/** Port pin for D- data line. */
#define ADK_USB_DMINUS_PIN  2
#endif /* ADK_USB_DMINUS_PIN */

/* AVR_USB_DEBUG macro enables debug functionality. */
#ifdef AVR_USB_DEBUG

/** Port for output of debug token. Least significant four bits are used. */
#ifndef ADK_USB_DEBUG_PORT
#define ADK_USB_DEBUG_PORT  B
#endif /* ADK_USB_DEBUG_PORT */

#endif /* AVR_USB_DEBUG */

#ifndef ADK_USB_DEVICE_CLASS
/** Device class code for device descriptor. Vendor-specific by default. */
#define ADK_USB_DEVICE_CLASS    0xff
#endif /* ADK_USB_DEVICE_CLASS */

#ifndef ADK_USB_DEVICE_SUBCLASS
/** Device subclass code for device descriptor. */
#define ADK_USB_DEVICE_SUBCLASS 0
#endif /* ADK_USB_DEVICE_SUBCLASS */

#ifndef ADK_USB_VENDOR_ID
/** Vendor ID for device descriptor. */
#define ADK_USB_VENDOR_ID       0xbeef //XXX
#endif /* ADK_USB_VENDOR_ID */

#ifndef ADK_USB_PRODUCT_ID
/** Product ID for device descriptor. */
#define ADK_USB_PRODUCT_ID      0xbeef //XXX
#endif /* ADK_USB_PRODUCT_ID */

#ifndef ADK_USB_VERSION
/** Device version (BCD) for device descriptor. */
#define ADK_USB_VERSION         0x0100
#endif /* ADK_USB_VERSION */

/* The following strings may be defined in the configuration:
 *  ADK_USB_MANUFACTURER_STRING
 *  ADK_USB_PRODUCT_STRING
 *  ADK_USB_SERIAL_STRING
 * If the macro is not defined the corresponding string is not present.
 */

#ifndef ADK_USB_POWER_CONSUMPTION
/** Device power consumption in mA. */
#define ADK_USB_POWER_CONSUMPTION           100
#endif /* ADK_USB_POWER_CONSUMPTION */

/* Internal definitions. */

/** PORTx register for data port. */
#define ADK_USB_DPORT_PORT      __CONCAT(PORT, ADK_USB_DATA_PORT)
/** DDRx register for data port. */
#define ADK_USB_DPORT_DDR       __CONCAT(DDR, ADK_USB_DATA_PORT)
/** PINx register for data port. */
#define ADK_USB_DPORT_PIN       __CONCAT(PIN, ADK_USB_DATA_PORT)

#ifdef AVR_USB_DEBUG

/** PORTx register for debug port. */
#define ADK_USB_DBGPORT_PORT    __CONCAT(PORT, ADK_USB_DEBUG_PORT)
/** DDRx register for debug port. */
#define ADK_USB_DBGPORT_DDR     __CONCAT(DDR, ADK_USB_DEBUG_PORT)

#endif /* AVR_USB_DEBUG */

#endif /* USB_CONFIG_INT_H_ */
