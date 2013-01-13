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
    AdkUsbInterrupt();
    /* Reset pending interrupt flag. */
    AVR_BIT_SET8(EIFR, INTF0);

    //XXX
    if (!AVR_BIT_GET8(GIMSK, INT0)) {
        _delay_ms(1000.0);
        AVR_USB_DBG_SET(0xf);
        _delay_ms(1000.0);
        AVR_USB_DBG_SET(adkUsbRxBuf[0] & 0xf);
        _delay_ms(1000.0);
        AVR_USB_DBG_SET(0xf);
        _delay_ms(1000.0);
        AVR_USB_DBG_SET(adkUsbRxBuf[0] >> 4);
    }
}

int
main(void)
{
    AdkUsbSetup();

    /* Interrupt by low level - bus activity on D- line. */
    AVR_BIT_CLR8(MCUCR, ISC01);
    AVR_BIT_CLR8(MCUCR, ISC00);
    AVR_BIT_SET8(GIMSK, INT0);

    sei();

    while (1) {
        AdkUsbPoll();
    }

    return 0;
}
