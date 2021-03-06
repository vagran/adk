/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
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

/* Buffer for echoing received packets. */
static u8 echoBuffer[ADK_USB_MAX_DATA_SIZE];

/* Provide callback for received data. */
bool_t
AdkUsbOnReceive(u8 *data, u8 size)
{
    memcpy(echoBuffer, data, size);
    return TRUE;
}

/* Provide callback for transmitted data. */
u8
AdkUsbOnTransmit(u8 size)
{
    adkUsbUserTxData.ram_ptr = echoBuffer;
    return size;
}

int
main(void)
{
    AdkUsbInit();

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
