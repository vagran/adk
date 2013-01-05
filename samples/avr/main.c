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

int
main(void)
{
    DDRD = (1 << PD6);

    sei();

    while (1) {
        /* Toggle PORTD.6 bit. */
        PIND = (1 << PD6);
        for (volatile u32 i = 0; i < 100000; i++);
    }

    return 0;
}
