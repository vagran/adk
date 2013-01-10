/* /ADK/src/libavr/usb/usb_tranceiver.S
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file usb_tranceiver.S
 * Time-critical low-level code for USB interface implementation.
 */

#include <adk.h>

/** Zero register. */
#define ZERO                 r1

/** Mask for D+ and D- lines. */
#define DATA_LINES_MASK     (_BV(AVR_USB_DPLUS_PIN) | _BV(AVR_USB_DMINUS_PIN))
/** Masked value for J state. */
#define DATA_LINES_J        _BV(AVR_USB_DPLUS_PIN)
/** Masked value for K state. */
#define DATA_LINES_K        _BV(AVR_USB_DMINUS_PIN)
/** Masked value for idle state. */
#define DATA_LINES_IDLE     DATA_LINES_J
/** Masked value for SE0 (single-ended zero) state. */
#define DATA_LINES_SE0      0
/** Masked value for SE1 (single-ended one) state. */
#define DATA_LINES_SE1      (_BV(AVR_USB_DPLUS_PIN) | _BV(AVR_USB_DMINUS_PIN))

/** Wait for one cycle. */
#define WAIT_1                nop
/** Wait for two cycles. */
#define WAIT_2                nop; nop
/** Wait for three cycles. */
#define WAIT_3                nop; nop; nop
/** Wait for four cycles. */
#define WAIT_4                push r0; pop r0
/** Wait for five cycles. */
#define WAIT_5                push r0; pop r0; nop
/** Wait for six cycles. */
#define WAIT_6                push r0; pop r0; nop; nop
/** Wait for seven cycles. */
#define WAIT_7                push r0; pop r0; nop; nop; nop
/** Wait for eight cycles. */
#define WAIT_8                push r0; pop r0; push r0; pop r0

/* All the physical communications over USB bus are handled in this function.
 * The code is intented to be used with 20MHz crystal. So the timing is 13.33
 * CPU cycles per bit.
 */
.global AdkUsbInterrupt
.type   AdkUsbInterrupt, @function
AdkUsbInterrupt:
    /* Before starting packet processing we must check if the device requires
     * reset (if it is in POWERED state). If yes, we should not process anything
     * but reset signal (SE0 >= 10ms).
     */
    lds r18, adkUsbState
    andi r18, ADK_USB_STATE_MASK
    breq powered_state_process

    /* Next we should check if interrupt is caused by SE0 or differential signal.
     * SE0 indicates possible reset.
     */
    in r18, AVR_USB_DPORT_PIN
    andi r18, DATA_LINES_MASK
    breq se0_process

    /* First thing which must be done is synchronizing with SYNC pattern. For
     * low-speed function it must be KJKJKJKK. It is not known when this
     * function is entered relatively to the SYNC pattern position because of
     * interrupt handler calling delay and ISR prologue. It is only must be
     * guaranteed that it is not later than last the J token which is plenty of
     * time (66 cycles). Only D+ line is checked assuming that diferential
     * signal is applied.
     */

    /* Firstly skip J token. */
sync_skip_J:
    sbis AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN
    rjmp sync_skip_J
    /* K token detected. It is either K or KK which ends the SYNC pattern. Skip
     * 1.5 bits (20 cycles) up to the middle of the second K token. If it is
     * still K, skip one more bit (13 cycles) up to the middle of the first data
     * bit. Two cycles already consumed by the code above. Digits in square
     * brackets indicate number of cycles skipped from the previous bit after
     * the instruction executed.
     */
    ret

    ldi r18, 4 // [3] 4 cycles per iteration below times 4 is 15 (minus one
               // cycle for the last non-fired jump) cycles
sync_skip_K:
    dec r18 // [4]
    WAIT_1 // [5]
    brne sync_skip_K // [7]
    // [18]
    WAIT_2 // [20]
    /* Check if we still have K token. */
    sbis AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN // [1]
    rjmp sync_skip_J // [2] It was KJ, continue SYNC pattern skipping

    /* Skip 13 cycles up to the middle of the first data bit. */
    ldi r18, 3 // [3]
sync_skip_first_bit:
    dec r18 // [4]
    brne sync_skip_first_bit // [6]
    // [11]
    WAIT_2 // [13]

    /** Here KK token is skipped and we are precisely in the middle of the first
     * data bit.
     */
    //XXX
    ret

    /* Device is in POWERED state. Skip anything but reset signal. */
powered_state_process:
    in r18, AVR_USB_DPORT_PIN
    andi r18, DATA_LINES_MASK
    breq se0_process
    /* Not a reset signal, skip the packet. */

    /* Skip the remaining packet data, i.e. wait until EOP token (SE0 longer
     * than one bit 13 cycles followed by J state).
     */
skip_packet:
    in r18, AVR_USB_DPORT_PIN
    andi r18, DATA_LINES_MASK
    brne skip_packet // [3]
    //XXX


se0_process:
    //XXX
