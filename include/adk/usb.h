/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file usb.h
 * Support for AVR USB devices.
 */

#ifndef USB_H_
#define USB_H_

/** Custom request for transferring data from host to device. */
#define ADK_USB_REQ_ADK_WRITE           0xf0
/** Custom request for transferring data from device to host. */
#define ADK_USB_REQ_ADK_READ            0xf1

#ifdef ADK_PLATFORM_AVR
#include <adk/avr/usb.h>
#else
#include <adk/libusb.h>
#endif /* ADK_PLATFORM_AVR */

#endif /* USB_H_ */
