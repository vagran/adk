/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file usb_config.h
 * User overwrites of default USB interface configuration values.
 */

#ifndef USB_CONFIG_H_
#define USB_CONFIG_H_

#include "app_config.h"

/* Define some device strings. */
#define ADK_USB_MANUFACTURER_STRING         "Artyom Lebedev"
#define ADK_USB_PRODUCT_STRING              "ADK I/O"

#include <adk/avr/usb_config.h>

#endif /* USB_CONFIG_H_ */
