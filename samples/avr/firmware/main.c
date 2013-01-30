/* /ADK/samples/avr/firmware/main.c
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
    EIFR = _BV(INTF0);
}

/* Provide callback for received data. */
bool_t
AdkUsbOnReceive(u8 *data, u8 size)
{
    PORTB = *data & 0xf;
    return TRUE;
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
