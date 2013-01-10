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

u8 intr_cnt;
ISR(INT0_vect)
{
    AVR_USB_DBG_SET((intr_cnt + 1) * 4);//XXX
    AdkUsbInterrupt();
    intr_cnt++;
    //XXX disable further interrupts until the first packet processing is fully debugged
    if (intr_cnt == 2) {
        AVR_BIT_CLR8(GIMSK, INT0);
    }
}

int
main(void)
{
    AdkUsbSetup();

    /* Interrupt by falling edge - SYNC pattern start on D- line. */
    AVR_BIT_SET8(MCUCR, ISC01);
    AVR_BIT_CLR8(MCUCR, ISC00);
    AVR_BIT_SET8(GIMSK, INT0);

    sei();

    while (1) {
        AdkUsbPoll();
    }

    return 0;
}
