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
/* adkUsbRxState variable copy. */
#define RX_STATE            r23

/* Variables below used after the packet is received. */

/* PID of the received packet. */
#define RX_PID              r18
#define STATE               r22

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

/* Set new state. */
.macro m_SetState state
    andi STATE, ~ADK_USB_STATE_MASK
    ori STATE, \state
.endm

/* Transmit the provided packet. The provided data should include SYNC pattern.
 * This function is not supposed to be used from C. It does not correct the leap
 * cycle because 13 cycles per bit is still in the acceptable rate range (per
 * the specification), host receiver synchronizes with edges anyway.
 */
/* Arguments. */
#define TRANS_DATA          X
#define TRANS_DATA_LO       r26
#define TRANS_DATA_HI       r27
#define TRANS_LEN           r18
/* Variables. */
#define TRANS_STUFF_CNT     r19
#define TRANS_BYTE          r20
#define TRANS_BIT_CNT       r21

.global AdkUsbTrasmit
.type   AdkUsbTrasmit, @function
AdkUsbTrasmit:
    ldi TRANS_STUFF_CNT, 6
    ldi TRANS_BIT_CNT, 0xff // Carry each eight iterations.
    ld TRANS_BYTE, TRANS_DATA+
    ldi TMP0, DATA_LINES_MASK

    /* Drive the data lines. */
    sbi _SFR_IO_ADDR(AVR_USB_DPORT_PORT), AVR_USB_DMINUS_PIN
    sbi _SFR_IO_ADDR(AVR_USB_DPORT_DDR), AVR_USB_DPLUS_PIN
    sbi _SFR_IO_ADDR(AVR_USB_DPORT_DDR), AVR_USB_DMINUS_PIN

    /* Numbers in square brackets are number of cycles since last bit front edge. */

trans_loop: // [10]
    lsr TRANS_BYTE // [11]
    brcs trans_1 // [12]
    /* Transmit bit zero. */
    ldi TRANS_STUFF_CNT, 6 // [13] Reset stuffing counter.
    out _SFR_IO_ADDR(AVR_USB_DPORT_PIN), TMP0 // [1] Toggle data lines.
    WAIT_1 // [2]
    subi TRANS_BIT_CNT, 0x20 // [3] Carry each eight iterations.
    brcc trans_same_byte // [4]
    dec TRANS_LEN // [5]
    breq trans_EOP // [6]
    ld TRANS_BYTE, TRANS_DATA+ // [8]
    rjmp trans_loop // [10]
trans_1: // [13]
    /* Transmit bit one. */
    dec TRANS_STUFF_CNT // [1]
    breq trans_stuff // [2]
trans_check_byte:
    subi TRANS_BIT_CNT, 0x20 // [3] Carry each eight iterations.
    brcc trans_same_byte // [4]
trans_next_byte: // [4]
    dec TRANS_LEN // [5]
    breq trans_EOP // [6] All bytes transmitted.
    ld TRANS_BYTE, TRANS_DATA+ // [8]
    rjmp trans_loop // [10]
trans_same_byte: // [5]
    WAIT_3 // [8]
    rjmp trans_loop // [10]
trans_stuff: // [3]
    ldi TRANS_STUFF_CNT, 6 // [4] Reset stuffing counter.
    WAIT_8 // [12]
    subi TRANS_BIT_CNT, 0x20 // [13] Carry each eight iterations.
    /* Transmit stuff bit. */
    out _SFR_IO_ADDR(AVR_USB_DPORT_PIN), TMP0 // [1] Toggle data lines.
    WAIT_1 // [2]
    brcs trans_next_byte // [3]
    rjmp trans_same_byte // [5]

trans_EOP: // [7]
    /* Low level for both lines. */
    in TMP0, _SFR_IO_ADDR(AVR_USB_DPORT_PORT) // [8]
    andi TMP0, ~DATA_LINES_MASK // [9]
    WAIT_4 // [13]
    out _SFR_IO_ADDR(AVR_USB_DPORT_PORT), TMP0 // [1]
    /* Wait for approximately two bits (26 cycles). */
    ldi TMP0, 8 // [2]
trans_EOP_loop:
    dec TMP0 // [3]
    brne trans_EOP_loop // [5]

    /* Drive to J state. */
    sbi _SFR_IO_ADDR(AVR_USB_DPORT_PORT), AVR_USB_DMINUS_PIN
    /* Switch lines back to receiving. */
    cbi _SFR_IO_ADDR(AVR_USB_DPORT_DDR), AVR_USB_DMINUS_PIN
    cbi _SFR_IO_ADDR(AVR_USB_DPORT_DDR), AVR_USB_DPLUS_PIN
    cbi _SFR_IO_ADDR(AVR_USB_DPORT_PORT), AVR_USB_DMINUS_PIN
    ret

/* Arguments for AdkUsbCrc16() function. Per GCC calling convention. */
#define CRC_DATA_ARG_LO     r24
#define CRC_DATA_ARG_HI     r25
#define CRC_LEN_ARG         r22
#define CRC_RESULT_LO       r24
#define CRC_RESULT_HI       r25
/* Variables for AdkUsbCrc16() function. */
#define CRC_DATA            X
#define CRC_DATA_LO         r26
#define CRC_DATA_HI         r27
#define CRC_X               r23
#define CRC_Y_LO            r20
#define CRC_Y_HI            r21

.global AdkUsbCrc16
.type   AdkUsbCrc16, @function
AdkUsbCrc16:
    /* See Crc16() function in usb_capture_analyzer.py for the algorithm
     * reference.
     */
    movw CRC_DATA_LO, CRC_DATA_ARG_LO
    ldi CRC_RESULT_LO, 0xff
    ldi CRC_RESULT_HI, 0xff
    inc CRC_LEN_ARG
crc_loop:
    dec CRC_LEN_ARG
    breq crc_exit

    ld CRC_X, CRC_DATA+
    eor CRC_X, CRC_RESULT_LO

    clr CRC_Y_LO
    mov CRC_Y_HI, CRC_X // Y = X << 8
    lsr CRC_Y_HI
    ror CRC_Y_LO // Y = X << 7
    mov CRC_RESULT_LO, CRC_Y_HI // Use CRC_RESULT_LO as temporal register
    lsr CRC_RESULT_LO
    eor CRC_Y_HI, CRC_RESULT_LO
    mov CRC_RESULT_LO, CRC_Y_LO
    ror CRC_RESULT_LO
    eor CRC_RESULT_LO, CRC_Y_LO // Y = (X << 7) ^ (X << 6) // Ylo is RESULTlo
    eor CRC_RESULT_LO, CRC_RESULT_HI
    mov CRC_RESULT_HI, CRC_Y_HI // RESULT = (X << 7) ^ (X << 6) ^ (resid >> 8)

    /* Get parity of X. */
    mov CRC_Y_LO, CRC_X
    swap CRC_Y_LO
    eor CRC_X, CRC_Y_LO
    mov CRC_Y_LO, CRC_X
    lsr CRC_Y_LO
    lsr CRC_Y_LO
    eor CRC_X, CRC_Y_LO
    /* Parity now is in least signinficant two bits of X. Now do the trick -
     * increment X and check resulted parity in bit 1. It is equivalent to do
     * XOR on these two bits.
     */
    inc CRC_X
    andi CRC_X, 2
    breq crc_loop
    /* Parity set, apply XOR 0xc001 to result. */
    ldi CRC_X, 0x01
    eor CRC_RESULT_LO, CRC_X
    ldi CRC_X, 0xc0
    eor CRC_RESULT_HI, CRC_X
    rjmp crc_loop
crc_exit:
    com CRC_RESULT_LO
    com CRC_RESULT_HI
    ret

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
    lds RX_STATE, adkUsbRxState // [10]
    sbrc RX_STATE, ADK_USB_RX_CUR_BUF_BIT // [11]
    rjmp use_rx_buf1 // [12]
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
    rjmp skip_packet
    /* EOP received in the middle of byte. */
handle_invalid_EOP:
    rjmp skip_packet_se0

    /* EOP seen when the byte is received. */
handle_EOP:
    dec RX_BUF_SIZE
    subi RX_BUF_SIZE, ADK_USB_RX_BUF_SIZE
    neg RX_BUF_SIZE
    sub RX_BUF_LO, RX_BUF_SIZE
    sbci RX_BUF_HI, 0
    ld RX_PID, RX_BUF+
    /* Here we have an assumption that EOP was seen and not reset. If it was a
     * token the next packet (if any) will follow shortly (typically 3-4 bits
     * interval). So we must quickly check the received packet and take
     * approriate actions. Response should be sent in 6.5 bits time (about 100
     * cycles accounting also current EOP which should not yet end). If there
     * is next packet which should be sent by a host, then it will typically
     * sent in 60 cycles.
     *
     * First thing to do is check RX_MINE flag. If it is not set then process
     * only tokens which match our address.
     */
    lds STATE, adkUsbState
    /* Check if it is token to us. */
    cpi RX_PID, ADK_USB_PID_SETUP
    breq token_found
    cpi RX_PID, ADK_USB_PID_IN
    breq token_found
    cpi RX_PID, ADK_USB_PID_OUT
    breq token_found
    /* Not a token, check if current transaction is our one. */
    sbrc RX_STATE, ADK_USB_RX_MINE_BIT
    rjmp state_machine_enter
    /* Not our packet. */
    rjmp check_idle
token_found:
    /* Check destination address. */
    ld TMP0, RX_BUF
    andi TMP0, 0x7f
    lds TMP1, adkUsbDeviceAddress
    cp TMP0, TMP1
    breq transaction_start
    /* Transaction for other device started. Clear RX_MINE for a case. */
    andi RX_STATE, ~ADK_USB_RX_MINE
    rjmp sm_exit
transaction_start:
    /* Have a packet for us, set RX_MINE and process through the state machine. */
    ori RX_STATE, ADK_USB_RX_MINE
    /* SETUP token always sets current state to SETUP. */
    cpi RX_PID, ADK_USB_PID_SETUP
    breq sm_listen
state_machine_enter:
    mov TMP0, STATE
    andi TMP0, ADK_USB_STATE_MASK
    /* Get address for vector of this state. Assuming POWERED state cannot
     * reach this point.
     */
    ldi ZL, AVR_LO8(sm_jump_table)
    ldi ZH, AVR_HI8(sm_jump_table)
    dec TMP0 // Not accounting POWERED state.
    lsr ZH
    ror ZL
    add ZL, TMP0
    adc ZH, ZERO
    ijmp
    /* Jump table for state handlers. Vectors for each state. Keep in sync with
     * ADK_USB_STATE_XXX constants.
     */
sm_jump_table:
    rjmp sm_listen
    rjmp sm_setup
    rjmp sm_write_data
    rjmp sm_write_status
    rjmp sm_read_data

/* State handlers. */

/************************* LISTEN *********************************************/

sm_listen:
    /* Accept only SETUP tokens in this state. */
    ldi TMP0, ADK_USB_PID_SETUP
    cpse RX_PID, TMP0
    rjmp send_stall
    m_SetState ADK_USB_STATE_SETUP
    rjmp sm_exit

/************************* SETUP **********************************************/

sm_setup:
    cpi RX_PID, ADK_USB_PID_DATA0
    brne sm_s_not_data0
    sbrc RX_STATE, ADK_USB_RX_SETUP_BIT
    rjmp send_nak
    /* DATA0, RX_SETUP not set. Hold data in the receiving buffer for further
     * processing.
     */
    ori RX_STATE, ADK_USB_RX_SETUP
    subi RX_BUF_SIZE, 3 // Not account PID and CRC in length
    /* Assume buffer is currently free so the size field is already zero. */
    or RX_STATE, RX_BUF_SIZE
    ldi TMP0, ADK_USB_RX_CUR_BUF
    eor RX_STATE, TMP0 // Toggle current active buffer.
    rjmp send_ack
sm_s_not_data0:
    /* Tokens other than DATA0. Accept only OUT token, NAK for the rest ones. */
    ldi TMP0, ADK_USB_PID_OUT
    cpse RX_PID, TMP0
    rjmp send_nak
    rjmp sm_exit

/************************* WRITE_DATA *****************************************/

sm_write_data:
    //XXX
    rjmp sm_exit

/************************* WRITE_STATUS ***************************************/

sm_write_status:
    /* IN: send status.
     * ACK: accept new address if any, advance to LISTEN.
     */
    cpi RX_PID, ADK_USB_PID_IN
    brne sm_ws_not_in
    rjmp send_status
sm_ws_not_in:
    cpi RX_PID, ADK_USB_PID_ACK
    brne sm_ws_not_ack
    /* Check if new address assigned. */
    lds TMP0, adkUsbNewDeviceAddress
    andi TMP0, 0x7f
    breq sm_ws_no_address
    /* Have new address. */
    sts adkUsbDeviceAddress, TMP0
    sts adkUsbNewDeviceAddress, ZERO
sm_ws_no_address:
    m_SetState ADK_USB_STATE_LISTEN
    andi RX_STATE, ~ADK_USB_RX_MINE
    rjmp sm_exit
sm_ws_not_ack:
    rjmp send_stall

/************************* READ_DATA ******************************************/

sm_read_data:
    lds TMP1, adkUsbTxState
    cpi RX_PID, ADK_USB_PID_IN
    brne sm_rd_not_in
    /* IN token received. */
    mov TRANS_LEN, TMP1
    andi TRANS_LEN, ADK_USB_TX_SIZE_MASK
    brne sm_rd_send_data
    /* Request more data. */
    ori STATE, ADK_USB_STATE_READ_WAIT
    rjmp send_nak
sm_rd_send_data:
    ldi TRANS_DATA_LO, AVR_LO8(adkUsbTxDataBuf)
    ldi TRANS_DATA_HI, AVR_HI8(adkUsbTxDataBuf)
    rcall AdkUsbTrasmit
    /* Transmission buffer released only after host acknowledgement. */
    rjmp sm_exit
sm_rd_not_in:
    cpi RX_PID, ADK_USB_PID_ACK
    brne sm_rd_not_ack
    /* ACK received, free transmission buffer. */
    andi TMP1, ~ADK_USB_TX_SIZE_MASK
    sts adkUsbTxState, TMP1
    andi RX_STATE, ~ADK_USB_RX_MINE
    rjmp sm_exit
sm_rd_not_ack:
    cpi RX_PID, ADK_USB_PID_DATA1
    brne sm_exit // Should be OUT token, no action
    /* Send ACK on DATA1 and end the transaction - status stage complete. */
    m_SetState ADK_USB_STATE_LISTEN
    rjmp send_ack

/******************************************************************************/

sm_exit:
    /* Save RX state. */
    sts adkUsbRxState, RX_STATE
    sts adkUsbState, STATE
    rjmp check_idle

    /* Send current transaction status. If transaction is not yet complete
     * (ADK_USB_STATE_TRANS_ACTIVE set) send NAK. If transaction failed
     * (ADK_USB_STATE_TRANS_FAILED set) send stall, reset failed indication and
     * advance state to LISTEN. If transaction completed sucessfully send empty
     * DATA1 packet.
     */
send_status:
    sbrc STATE, ADK_USB_STATE_TRANS_ACTIVE_BIT
    rjmp send_nak // Transaction not yet complete.
    sbrc STATE, ADK_USB_STATE_TRANS_FAILED_BIT
    rjmp send_status_failed
    /* Transaction completed successfully, send empty DATA1 packet. */
    ldi TMP0, ADK_USB_PID_DATA1
    sts adkUsbTxAuxBuf + 1, TMP0
    ldi TRANS_LEN, 4
    ldi TRANS_DATA_LO, AVR_LO8(adkUsbTxAuxBuf)
    ldi TRANS_DATA_HI, AVR_HI8(adkUsbTxAuxBuf)
    rcall AdkUsbTrasmit
    rjmp sm_exit
send_status_failed:
    /* Transaction failed, reset indication, andvance state to LISTEN and send
     * STALL.
     */
    andi STATE, ~ADK_USB_STATE_TRANS_FAILED
    m_SetState ADK_USB_STATE_LISTEN
    rjmp send_stall

/* Send ACK and exit. */
send_ack:
    ldi TMP0, ADK_USB_PID_ACK
    rjmp send_handshake
/* Send NAK and exit. */
send_nak:
    ldi TMP0, ADK_USB_PID_NAK
    rjmp send_handshake
/* Send STALL and exit. */
send_stall:
    ldi TMP0, ADK_USB_PID_STALL
    /* Current transaction complete, send handshake (PID in TMP0), reset RX_MINE
     * and check for next packet.
     */
send_handshake:
    sts adkUsbTxAuxBuf + 1, TMP0
    ldi TRANS_LEN, 2
    ldi TRANS_DATA_LO, AVR_LO8(adkUsbTxAuxBuf)
    ldi TRANS_DATA_HI, AVR_HI8(adkUsbTxAuxBuf)
    rcall AdkUsbTrasmit
    /* Reset RX_MINE. */
    andi RX_STATE, ~ADK_USB_RX_MINE
    rjmp sm_exit

    /* Device is in POWERED state. Skip anything but reset signal. */
powered_state_process:
    in TMP0, _SFR_IO_ADDR(AVR_USB_DPORT_PIN)
    andi TMP0, DATA_LINES_MASK
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
     * reentered. Wait for a packet for 8 bits (107 cycles). The loop time
     * should be less than bit length.
     */
check_idle:
    ldi TMP0, 11
check_idle_loop:
    in TMP1, _SFR_IO_ADDR(AVR_USB_DPORT_PIN) // [1]
    andi TMP1, DATA_LINES_MASK // [2]
    cpi TMP1, DATA_LINES_IDLE // [3]
    breq check_idle_inactive // [5]
    rjmp AdkUsbInterrupt // [6] Line active, handle it.
check_idle_inactive:
    dec TMP0 // [7]
    brne check_idle_loop // [9]
    /* Timeout exceeded while the line is still idle. Can return now. */
    ret
