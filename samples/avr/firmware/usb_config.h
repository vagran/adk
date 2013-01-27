/* /ADK/samples/avr/firmware/usb_config.h
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file usb_config.h
 * User overwrites of default USB interface configuration values.
 */

#ifndef USB_CONFIG_H_
#define USB_CONFIG_H_

#include "app_config.h"

/* Define some device strings. */
#define AVR_USB_MANUFACTURER_STRING         "Artyom Lebedev"
#define AVR_USB_PRODUCT_STRING              "ADK I/O"

#include <adk/avr/usb_config.h>

#endif /* USB_CONFIG_H_ */
