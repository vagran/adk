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

static void
DumpData(u8 *data, u8 size)
{
    for (u8 idx = 0; idx < size * 2; idx++) {
        u8 v = 0;
        for (u8 i = 0; i < 4; i++) {
            v = (v << 1) + 1;
            AVR_USB_DBG_SET(v);
            _delay_ms(500);
        }
        v = data[idx / 2];
        if (idx & 1) {
            AVR_USB_DBG_SET(v >> 4);
        } else {
            AVR_USB_DBG_SET(v & 0xf);
        }
        _delay_ms(2000);
    }
}

ISR(INT0_vect)
{
    static u8 count;
    AdkUsbInterrupt();
    /* Reset pending interrupt flag. */
    EIFR = _BV(INTF0);
    count++;
    //XXX
    if (!AVR_BIT_GET8(GIMSK, INT0)) {
        DumpData(&count, 1);
        DumpData(&adkUsbRxSize, 1);
        DumpData(adkUsbRxBuf, 3);
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
