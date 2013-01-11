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
.macro WAIT_1
    nop
.endm
/** Wait for two cycles. */
.macro WAIT_2
    nop
    nop
.endm
/** Wait for three cycles. */
.macro WAIT_3
    nop
    nop
    nop
.endm
/** Wait for four cycles. */
.macro WAIT_4
    push r0
    pop r0
.endm
/** Wait for five cycles. */
.macro WAIT_5
    push r0
    pop r0
    nop
.endm
/** Wait for six cycles. */
.macro WAIT_6
    push r0
    pop r0
    nop
    nop
.endm
/** Wait for seven cycles. */
.macro WAIT_7
    push r0
    pop r0
    nop
    nop
    nop
.endm
/** Wait for eight cycles. */
.macro WAIT_8
    push r0
    pop r0
    push r0
    pop r0
.endm

/* Receiver loop variables. */
/* Number of consecutive ones left before unstaff bit. */
#define ONES_CNT            r18
/* Pointer to receiving      buffer. */
#define RX_BUF              X
#define RX_BUF_LO           r26
#define RX_BUF_HI           r27
/* Number of bytes left in the receiving buffer. */
#define RX_BUF_SIZE         r19
/* Byte which is currently being received is accumulated in this register. */
#define RX_BYTE             r20
/* Previous state of the line. */
#define LAST_STATE          r21
/* Every leap bit has additional cycle. */
#define LEAP                r22

/** Value to substract from leap counter. */
#define LEAP_VALUE          (0x101 / 3)

/* Temporal register 0. */
#define TMP0                r24
/* Temporal register 1. */
#define TMP1                r25

.macro m_AVR_USB_DBG_SET token
    in TMP0, AVR_USB_DBGPORT_PORT
    andi TMP0, 0xf0
    ori TMP0, \token & 0x0f
    out AVR_USB_DBGPORT_PORT, TMP0
.endm

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
    lds TMP0, adkUsbState
    andi TMP0, ADK_USB_STATE_MASK
    brne above_powered_state
    rjmp powered_state_process
above_powered_state:

    /* Next we should check if interrupt is caused by SE0 or differential signal.
     * SE0 indicates possible reset.
     */
    in TMP0, AVR_USB_DPORT_PIN
    andi TMP0, DATA_LINES_MASK
    brne not_se0
    rjmp check_reset
 not_se0:

    /* First thing which must be done is synchronizing with SYNC pattern. For
     * low-speed function it must be KJKJKJKK. It is not known when this
     * function is entered relatively to the SYNC pattern position because of
     * interrupt handler calling delay and ISR prologue. It is only must be
     * guaranteed that it is not later than last the J token which is plenty of
     * time (66 cycles). Only D+ line is checked assuming that diferential
     * signal is applied.
     */

    /* Wait for J token. */
sync_wait_J:
    sbic AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN
    rjmp sync_wait_J
    /* Skip J token. */
sync_skip_J:
    /* Two cycles per sample in the code below - the maximally achievable
     * resolution value.
     */
    sbic AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN
    rjmp found_K
    /* J token is too long, skip the packet. */
    AVR_USB_DBG_SET(3)//XXX
    rjmp skip_packet

found_K: // [3]
    /* K token detected. It is either K or KK which ends the SYNC pattern. Skip
     * 1.5 bits (20 cycles) up to the middle of the second K token. If it is
     * still K, skip one more bit (13 cycles) up to the middle of the first data
     * bit. Two cycles already consumed by the code above. Digits in square
     * brackets indicate number of cycles skipped from the previous bit after
     * the instruction executed.
     */

    /* Set up receiver loop variables while skipping K token. */
    ldi ONES_CNT, 5 // [4] one bit already in the end of SYNC pattern.
    ldi RX_BUF_SIZE, ADK_USB_RX_BUF_SIZE // [5]
    ldi RX_BYTE, 0x80 // [6] When highest bit is shifted in the C flag, the whole byte is received
    /* Initialize to proper receive buffer. */
    ldi RX_BUF_LO, AVR_LO8(adkUsbRxBuf) // [7]
    ldi RX_BUF_HI, AVR_HI8(adkUsbRxBuf) // [8]
    lds LAST_STATE, adkUsbState // [10]
    andi LAST_STATE, ADK_USB_F_CUR_RX_BUF // [11]
    brne use_rx_buf1 // [12]
    /* Use RX buffer 0. */
    WAIT_1 // [13]
    rjmp rx_buf_initialized // [15]
use_rx_buf1: // [13]
    adiw RX_BUF_LO, ADK_USB_RX_BUF_SIZE // [15]
rx_buf_initialized:
    ldi LAST_STATE, DATA_LINES_K // [16]
    ldi LEAP, LEAP_VALUE // [17]
    WAIT_3 // [20]

    /* Check if we still have K token. */
    sbis AVR_USB_DPORT_PIN, AVR_USB_DPLUS_PIN // [1]
    rjmp sync_skip_J // [2] It was KJ, continue SYNC pattern skipping

    /* Skip 13 cycles up to the middle of the first data bit. */
    ldi TMP0, 3 // [3]
sync_skip_first_bit:
    dec TMP0 // [4]
    brne sync_skip_first_bit // [6]
    // [11]
    WAIT_2 // [13]

    /** Here KK token is skipped and we are precisely in the middle of the first
     * data bit.
     */
rx_loop:
    in TMP0, AVR_USB_DPORT_PIN // [1]
    andi TMP0, DATA_LINES_MASK // [2]
    breq handle_EOP // [3]
    cpse TMP0, LAST_STATE // [4]
    rjmp handle_0 // [6] Line state differs - bit zero received
    // [5] Bit one received.
    sec // [6]
    ror RX_BYTE // [7]
    brcs handle_byte_1 // [8]
    dec ONES_CNT // [9]
    breq handle_stuff_bit //[10]
    subi LEAP, LEAP_VALUE // [11]
    brcc rx_loop // [13] if branch taken, else [12]
    rjmp rx_loop // [14] (every leap bit)

    /* Handle received bit zero. */
handle_0: // [6]
    clc // [7]
    ror RX_BYTE // [8]
    brcs handle_byte_0 // [9]
    ldi ONES_CNT, 6 // [10]
    subi LEAP, LEAP_VALUE // [11]
    brcc rx_loop // [13] if branch taken, else [12]
    rjmp rx_loop // [14] (every leap bit)

    /* Handle received byte (last bit one). */
handle_byte_1: // [9]
    //XXX check for stuffing
    //XXX

    /* Handle received byte (last bit zero). */
handle_byte_0: // [10]
    //XXX
    //AVR_USB_DBG_SET(3)
    ret

handle_stuff_bit:
    //XXX
    //AVR_USB_DBG_SET(5)
    ret

handle_EOP:
    //XXX
    //AVR_USB_DBG_SET(6)
    ret

    /* Device is in POWERED state. Skip anything but reset signal. */
powered_state_process:
    in r18, AVR_USB_DPORT_PIN
    andi r18, DATA_LINES_MASK
    breq check_reset
    /* Not a reset signal, skip the packet. */

    /* Skip the remaining packet data, i.e. wait until EOP token (SE0 longer
     * than one bit 13 cycles followed by J state).
     */
skip_packet:
    in r18, AVR_USB_DPORT_PIN
    andi r18, DATA_LINES_MASK
    brne skip_packet // [3]
    /* SE0 detected. */
    ldi r19, 2 // [4]
skip_EOP:
    in r18, AVR_USB_DPORT_PIN // [5]
    andi r18, DATA_LINES_MASK // [6]
    brne skip_packet // [7]
    dec r19 // [8]
    brne skip_EOP // [10]
    /* SE0 longer than one bit. Now wait for J state but not longer than two
     * bits (26 cycles). Longer signal may be a possible reset.
     */
    ldi r18, 4 // [6]
wait_J:
    dec r18 // [7]
    breq check_reset // [8] Timeout exceeded, possible reset.
    sbis AVR_USB_DPORT_PIN, AVR_USB_DMINUS_PIN // [9]
    rjmp wait_J // [11]
    /* J state detected, assume line is idle now. */
    ret

    /* Check if reset signal is asserted. SE0 should be active longer than 10 ms
     * (200000 cycles).
     */
check_reset:
    ldi r18, AVR_LO8(20000)
    ldi r19, AVR_HI8(20000)
check_reset_loop:
    in r20, AVR_USB_DPORT_PIN // [1]
    andi r20, DATA_LINES_MASK // [2]
    brne reset_interrupted //[3]
    WAIT_1 // [4]
    subi r18, 1 // [5]
    sbci r19, 0 // [6]
    mov r0, r18 // [7]
    or r0, r19 // [8]
    brne check_reset_loop // [10]

    /* Reset confirmed, wait for bus idle and invoke the handler. */
wait_reset_finish:
    sbis AVR_USB_DPORT_PIN, AVR_USB_DMINUS_PIN
    rjmp wait_reset_finish
    rcall _AdkUsbOnReset

    /* SE0 is asserted less that required for reset. */
reset_interrupted:
    /* Nothing to do here. */
    ret
