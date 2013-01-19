/* /ADK/src/libavr/usb/usb.c
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file usb.c
 * USB interface software implementation.
 */

#include <adk.h>

u8 adkUsbState;

u8 adkUsbRxBuf[2 * ADK_USB_RX_BUF_SIZE];

u8 adkUsbRxState;

u8 adkUsbDeviceAddress;



void
AdkUsbSetup()
{
    /* Configure debug port if enabled. */
#   ifdef AVR_USB_DEBUG
    AVR_USB_DBGPORT_DDR |= 0x0f;
    AVR_USB_DBGPORT_PORT &= 0xf0;
#   endif /* AVR_USB_DEBUG */

    /* Configure data lines for input and disable pull-up resistors. */
    AVR_BIT_CLR8(AVR_USB_DPORT_DDR, AVR_USB_DPLUS_PIN);
    AVR_BIT_CLR8(AVR_USB_DPORT_DDR, AVR_USB_DMINUS_PIN);
    AVR_BIT_CLR8(AVR_USB_DPORT_PORT, AVR_USB_DPLUS_PIN);
    AVR_BIT_CLR8(AVR_USB_DPORT_PORT, AVR_USB_DMINUS_PIN);

    adkUsbState = 0;
    adkUsbDeviceAddress = 0;
    adkUsbRxState = 0;

    //XXX fill SYNC in transmission buffer
}

void
AdkUsbPoll()
{
    //XXX
    //check system requests
}

/** This function is called from assembler interrupt handler when reset is
 * detected on the line.
 */
void
_AdkUsbOnReset()
{
    adkUsbState = (adkUsbState & ~ADK_USB_STATE_MASK) | ADK_USB_STATE_LISTEN;
    adkUsbDeviceAddress = 0;
    adkUsbRxState = 0;
}
