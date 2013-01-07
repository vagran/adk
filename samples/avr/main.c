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

EMPTY_INTERRUPT(BADISR_vect)

int
main(void)
{
    PORTB = 0;
    DDRB = 0xf;

    AdkUsbSetup();

    //XXX
    DDRD = 0;
    //disable pull-ups
    PORTD = 0;
    //interrupt by falling edge
//    AVR_BIT_SET8(MCUCR, ISC11);
//    AVR_BIT_CLR8(MCUCR, ISC10);
//    AVR_BIT_SET8(GIMSK, INT1);

    //sei();

    while (1) {
        PORTB = (PORTB + 1) & (u8)0xf;
        for (volatile u32 i = 0; i < 100000; i++);
    }

    return 0;
}
