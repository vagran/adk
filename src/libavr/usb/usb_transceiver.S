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
#define DATA_LINES_J        _BV(AVR_USB_DMINUS_PIN)
/** Masked value for K state. */
#define DATA_LINES_K        _BV(AVR_USB_DPLUS_PIN)
/** Masked value for idle state. */
#define DATA_LINES_IDLE     DATA_LINES_J
/** Masked value for SE0 (single-ended zero) state. */
#define DATA_LINES_SE0      0
/** Masked value for SE1 (single-ended one) state. */
#define DATA_LINES_SE1      (_BV(AVR_USB_DPLUS_PIN) | _BV(AVR_USB_DMINUS_PIN))

#define TRIGGER sbi _SFR_IO_ADDR(PORTB), 0 //XXX

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
/* Pointer to receiving buffer. */
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
    in TMP0, _SFR_IO_ADDR(AVR_USB_DBGPORT_PORT)
    andi TMP0, 0xf0
    ori TMP0, \token & 0x0f
    out _SFR_IO_ADDR(AVR_USB_DBGPORT_PORT), TMP0
.endm

/* All the physical communications over USB bus are handled in this function.
 * The code is intented to be used with 20MHz crystal. So the timing is 13.333
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
    in TMP0, _SFR_IO_ADDR(AVR_USB_DPORT_PIN)
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
    sbic _SFR_IO_ADDR(AVR_USB_DPORT_PIN), AVR_USB_DPLUS_PIN
    rjmp sync_wait_J
    /* Skip J token. */
sync_skip_J:
    /* Two cycles per sample in the code below - the maximally achievable
     * resolution value.
     */
    sbic _SFR_IO_ADDR(AVR_USB_DPORT_PIN), AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic _SFR_IO_ADDR(AVR_USB_DPORT_PIN), AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic _SFR_IO_ADDR(AVR_USB_DPORT_PIN), AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic _SFR_IO_ADDR(AVR_USB_DPORT_PIN), AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic _SFR_IO_ADDR(AVR_USB_DPORT_PIN), AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic _SFR_IO_ADDR(AVR_USB_DPORT_PIN), AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic _SFR_IO_ADDR(AVR_USB_DPORT_PIN), AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic _SFR_IO_ADDR(AVR_USB_DPORT_PIN), AVR_USB_DPLUS_PIN
    rjmp found_K
    sbic _SFR_IO_ADDR(AVR_USB_DPORT_PIN), AVR_USB_DPLUS_PIN
    rjmp found_K
    /* J token is too long, probably false interrupt and line is idle. Double
     * check this by polling for idle state some additional time.
     */
    rjmp check_idle

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
    lds TMP0, adkUsbState // [10]
    andi TMP0, ADK_USB_F_CUR_RX_BUF // [11]
    brne use_rx_buf1 // [12]
    /* Use RX buffer 0. */
    WAIT_1 // [13]
    rjmp rx_buf_initialized // [15]

    /* Next part part of code inserted  here to overcome limitations of brXX
     * instructions - jump distance is limited to +/- 64 words.
     */

    /* EOP seen. */
hb1_handle_EOP:
    rjmp handle_EOP

hb1_rx_buffer_overflow:
    rjmp rx_buffer_overflow

hb1_handle_invalid_EOP:
    rjmp handle_invalid_EOP

hb1_stuff_error:
    rjmp stuff_error

hb1_handle_1: // [6]
    ldi RX_BYTE, 0xc0 // [7] Both flag bit and received bit at once
    dec RX_BUF_SIZE // [8]
    breq hb1_rx_buffer_overflow // [9]
    dec ONES_CNT // [10]
    breq hb1_handle_stuff_bit_1 // [11]
    /* Unfortunately here we do not have cycles to properly compensate leap. */
    subi LEAP, 2 * LEAP_VALUE // [12]
    rjmp rx_loop // [14]

    /* Handle stuff bit after the first bit of the next byte. */
hb1_handle_stuff_bit_1: // [12]
    /* Check line state a bit earlier. */
    in TMP0, _SFR_IO_ADDR(AVR_USB_DPORT_PIN) // [13]
    andi TMP0, DATA_LINES_MASK // [1]
    breq hb1_handle_invalid_EOP // [2]
    cp TMP0, LAST_STATE // [3]
    breq hb1_stuff_error // [4] Bit is not zero, stuffing error.
    mov LAST_STATE, TMP0 // [5]
    ldi ONES_CNT, 6 // [6]
    WAIT_6 // [12]
    /* Leap does not need some logic here because we processed 3 bits so leap
     * safely can be applied here unconditionaly and leap counter left
     * unmodified.
     */
    rjmp rx_loop // [14]

    /* Handle received byte (last bit one). */
handle_byte_1: // [9]
    st RX_BUF+, RX_BYTE // [11]
    dec ONES_CNT // [12]
    breq hb1_handle_stuff_bit // [13]
    in TMP0, _SFR_IO_ADDR(AVR_USB_DPORT_PIN) // [1]
    andi TMP0, DATA_LINES_MASK // [2]
    breq hb1_handle_EOP // [3]
    cp TMP0, LAST_STATE // [4]
    breq hb1_handle_1 // [6] Line state the same - bit one received
    // [5] Bit zero received.
    mov LAST_STATE, TMP0 // [6]
    ldi RX_BYTE, 0x40 // [7] Both flag bit and received bit at once
    ldi ONES_CNT, 6 // [8]
    dec RX_BUF_SIZE // [9]
    breq hb1_rx_buffer_overflow // [10]
    /* Leap cannot occur more than once in two bits so it is safe to compensate
     * here both bits.
     */
    subi LEAP, LEAP_VALUE * 2 // [11]
    brcc rx_loop // [13] if branch taken, else [12]
    rjmp rx_loop // [14] (every leap bit)

hb1_handle_stuff_bit: // [1]
    in TMP0, _SFR_IO_ADDR(AVR_USB_DPORT_PIN) // [2]
    andi TMP0, DATA_LINES_MASK // [3]
    breq hb1_handle_invalid_EOP // [4]
    /* Unfortunately we do not have enough spare cycles to check for stuffing
     * error...
     * So here we assume that zero bit is received.
     */
    mov LAST_STATE, TMP0 // [5]
    ldi RX_BYTE, 0x80 // [6]
    ldi ONES_CNT, 6 // [7]
    dec RX_BUF_SIZE // [8]
    breq hb1_rx_buffer_overflow // [9]
    WAIT_1 // [10]
    /* Leap cannot occur more than once in two bits so it is safe to compensate
     * here both bits.
     */
    subi LEAP, LEAP_VALUE * 2 // [11]
    brcc rx_loop // [13] if branch taken, else [12]
    rjmp rx_loop // [14] (every leap bit)

use_rx_buf1: // [13]
    adiw RX_BUF_LO, ADK_USB_RX_BUF_SIZE // [15]
rx_buf_initialized:
    ldi LAST_STATE, DATA_LINES_K // [16]
    ldi LEAP, LEAP_VALUE // [17]
    /* Skip less by three cycles because oscillograms show that in vast majority
     * of cases K token is detected with a delay.
     */

    /* Check if we still have K token. */
    sbis _SFR_IO_ADDR(AVR_USB_DPORT_PIN), AVR_USB_DPLUS_PIN // [1]
    rjmp sync_skip_J // [2] It was KJ, continue SYNC pattern skipping

    /* Skip 13 cycles up to the middle of the first data bit. */
    ldi TMP0, 3 // [3]
sync_skip_first_bit:
    dec TMP0 // [4]
    brne sync_skip_first_bit // [6]
    // [11]
    WAIT_2 // [13]
    TRIGGER//XXX
    /** Here KK token is skipped and we are precisely in the middle of the first
     * data bit.
     */
rx_loop:
    in TMP0, _SFR_IO_ADDR(AVR_USB_DPORT_PIN) // [1]
    andi TMP0, DATA_LINES_MASK // [2]
    breq handle_invalid_EOP // [3]
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
    mov LAST_STATE, TMP0 // [7]
    lsr RX_BYTE // [8]
    brcs handle_byte_0 // [9]
    ldi ONES_CNT, 6 // [10]
    subi LEAP, LEAP_VALUE // [11]
    brcc rx_loop // [13] if branch taken, else [12]
    rjmp rx_loop // [14] (every leap bit)

    /* Handle received byte (last bit zero). */
handle_byte_0: // [10]
    st RX_BUF+, RX_BYTE // [12]
    /* Check line state a bit earlier than center of bit. It should not affect
     * the result because we are still very close to the bit center.
     */
    WAIT_1 //XXX
    TRIGGER//XXX
    in TMP0, _SFR_IO_ADDR(AVR_USB_DPORT_PIN) // [13]
    andi TMP0, DATA_LINES_MASK // [1]
    breq handle_EOP // [2]
    cpse TMP0, LAST_STATE // [3]
    rjmp hb0_handle_0 // [4]
    /* Bit one received. */
    ldi RX_BYTE, 0xc0 // [5] Both flag bit and received bit at once
    ldi ONES_CNT, 5 // [6] Before that bit we received zero, so 5 ones still allowed
    dec RX_BUF_SIZE // [7]
    breq rx_buffer_overflow // [8]
    WAIT_2 // [10]
    /* Leap cannot occur more than once in two bits so it is safe to compensate
     * here both bits.
     */
    subi LEAP, LEAP_VALUE * 2 // [11]
    brcc rx_loop // [13] if branch taken, else [12]
    rjmp rx_loop // [14] (every leap bit)

hb0_handle_0: // [5]
    mov LAST_STATE, TMP0 // [6]
    ldi RX_BYTE, 0x40 // [7] Both flag bit and received bit at once
    ldi ONES_CNT, 6 // [8]
    dec RX_BUF_SIZE // [9]
    breq rx_buffer_overflow // [10]
    /* Leap cannot occur more than once in two bits so it is safe to compensate
     * here both bits.
     */
    subi LEAP, LEAP_VALUE * 2 // [11]
    brcc rx_loop // [13] if branch taken, else [12]
    rjmp rx_loop // [14] (every leap bit)

    /* Handle stuff bit which should be zero. */
handle_stuff_bit: // 11
    ldi ONES_CNT, 6 // [12]
    in TMP0, _SFR_IO_ADDR(AVR_USB_DPORT_PIN) // [13]
    andi TMP0, DATA_LINES_MASK // [1]
    breq handle_invalid_EOP // [2]
    cp TMP0, LAST_STATE // [3]
    breq stuff_error // [4] Bit is not zero, stuffing error.
    mov LAST_STATE, TMP0 // [5]
    WAIT_5 // [10]
    subi LEAP, LEAP_VALUE // [11]
    brcc rx_loop // [13] if branch taken, else [12]
    rjmp rx_loop // [14] (every leap bit)

    /* One is read on stuff bit position. */
stuff_error:

    /* Receiving buffer has been overflowed. */
rx_buffer_overflow:
    AVR_USB_DBG_SET(3)
    in      TMP0, _SFR_IO_ADDR(GIMSK) //XXX
    andi    TMP0, ~_BV(INT0) //XXX
    out     _SFR_IO_ADDR(GIMSK), TMP0 //XXX
    rjmp skip_packet

    /* EOP received in the middle of byte. */
handle_invalid_EOP:
    AVR_USB_DBG_SET(5)
    in      TMP0, _SFR_IO_ADDR(GIMSK) //XXX
    andi    TMP0, ~_BV(INT0) //XXX
    out     _SFR_IO_ADDR(GIMSK), TMP0 //XXX
    rjmp skip_EOP

   /* EOP seen when the byte is received. */
handle_EOP:
    ldi TMP0, 7 // [4] XXX
    out PORTB, TMP0 // [5] XXX
    dec RX_BUF_SIZE
    /* Here we have an assumption that EOP was seen and not reset. If it was a
     * token the next packet (if any) will follow shortly (typically 3-4 bits
     * interval). So we must quickly check the received packet and take
     * approriate actions.
     */
    //XXX
    AVR_USB_DBG_SET(7)
    in      TMP0, _SFR_IO_ADDR(GIMSK) //XXX
    andi    TMP0, ~_BV(INT0) //XXX
    out     _SFR_IO_ADDR(GIMSK), TMP0 //XXX
    ldi TMP0, ADK_USB_RX_BUF_SIZE
    sub TMP0, RX_BUF_SIZE
    sts adkUsbRxSize, TMP0
    //XXX check token, address, wait for data packet if applicable
    ret

    /* Device is in POWERED state. Skip anything but reset signal. */
powered_state_process:
    in r18, _SFR_IO_ADDR(AVR_USB_DPORT_PIN)
    andi r18, DATA_LINES_MASK
    breq check_reset
    /* Not a reset signal, skip the packet. */

    /* Skip the remaining packet data, i.e. wait until EOP token (SE0 longer
     * than one bit 13 cycles followed by J state). Also do not allow line to
     * be in idle state more than 6+ bits (100 cycles)
     */
    ldi TMP1, 10
skip_packet:
    in TMP0, _SFR_IO_ADDR(AVR_USB_DPORT_PIN) // [1]
    andi TMP0, DATA_LINES_MASK // [2]
    breq skip_packet_se0 // [3]
    cpi TMP0, DATA_LINES_IDLE // [4]
    breq skip_packet_idle // [5]
    /* Line active, reset timeout. */
    ldi TMP1, 10 // [6]
    WAIT_2 // [8]
    rjmp skip_packet // [10]
skip_packet_idle:  // [6]
    /* Line idle. */
    dec TMP1 // [7]
    breq check_idle // [8]
    rjmp skip_packet // [10]
skip_packet_se0:

    /* SE0 detected.
     * Now wait for J state but not longer than two bits (26 cycles). Longer
     * signal may be a possible reset.
     */
    ldi TMP0, 4 // [9]
wait_non_SE0:
    dec TMP0 // [10]
    breq check_reset // [11] Timeout exceeded, possible reset.
    in TMP1, _SFR_IO_ADDR(AVR_USB_DPORT_PIN) // [12]
    andi TMP1, DATA_LINES_MASK // [13]
    breq wait_non_SE0 // [15]
    /* Non-SE0 state detected, check if the line is idle now. */
    rjmp check_idle

    /* Check if reset signal is asserted. SE0 should be active longer than 2.5us
     * (50 cycles).
     */
check_reset:
    ldi TMP0, 8
check_reset_loop:
    in TMP1, _SFR_IO_ADDR(AVR_USB_DPORT_PIN) // [1]
    andi TMP1, DATA_LINES_MASK // [2]
    brne check_idle //[3]
    dec TMP0 // [4]
    brne check_reset_loop // [6]

    /* Reset confirmed, invoke the handler and wait for bus idle. */
    rcall _AdkUsbOnReset
wait_reset_finish:
    in TMP0, _SFR_IO_ADDR(AVR_USB_DPORT_PIN)
    andi TMP0, DATA_LINES_MASK
    breq wait_reset_finish
    /* Fall-through to be able to process packet which could immediately
     * follow the reset signal.
     */

    /* Ensure the line is idle for some time before exiting from interrupt
     * handler. Otherwise we could miss packet SYNC pattern if the is exited and
     * reentered. Wait for a packet for 10 bits (133 cycles). The loop time
     * should be less than bit length.
     */
check_idle:
    ldi TMP0, 19
check_idle_loop:
    in TMP1, _SFR_IO_ADDR(AVR_USB_DPORT_PIN) // [1]
    andi TMP1, DATA_LINES_MASK // [2]
    cpse TMP1, DATA_LINES_IDLE // [3]
    rjmp AdkUsbInterrupt // [4] Line active, handle it.
    dec TMP0 // [5]
    brne check_idle_loop // [7]
    /* Timeout exceeded while the line is still idle. Can return now. */
    ret
