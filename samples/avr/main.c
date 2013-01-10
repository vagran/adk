/* /ADK/samples/avr/main.c
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file main.c
 * TODO insert description here.
 */

#include <adk.h>

ISR(INT0_vect)
{
    AVR_USB_DBG_SET(1);//XXX
    AdkUsbInterrupt();
    //XXX disable further interrupts until the first packet processing is fully debugged
    AVR_BIT_CLR8(GIMSK, INT0);
}

int
main(void)
{
    AdkUsbSetup();

    /* Interrupt by falling edge - SYNC pattern start on D- line. */
    AVR_BIT_SET8(MCUCR, ISC01);
    AVR_BIT_CLR8(MCUCR, ISC00);
    AVR_BIT_SET8(GIMSK, INT0);

    //sei();

    while (1) {
        u8 b = AVR_BIT_GET8(AVR_USB_DPORT_PIN, AVR_USB_DMINUS_PIN) ? 1 : 0;
        b |= AVR_BIT_GET8(AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN) ? 2 : 0;
        AVR_USB_DBG_SET(b);
        //AdkUsbPoll();
    }

    return 0;
}
