/* /ADK/samples/avr/main.c
 *
 * This file is a part of ADK library.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
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
    DDRB = (1 << PB1);
    PORTB = 0;

    sei();
    return 0;
}
