/* /ADK/include/adk/avr/usb.h
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file usb.h
 * USB interface software implementation.
 */

#ifndef USB_H_
#define USB_H_

/* User application USB configuration file. Should be provided by the application.
 * It also should include <adk/avr/usb_config.h> file in the end of definitions.
 */
#include <usb_config.h>

#ifndef USB_CONFIG_INT_H_
#error "<adk/avr/usb_config.h> should be included by user usb_config.h file!"
#endif

#if ADK_MCU_FREQ != 20000000
#error "USB interface is supported only with 20MHz crystal!"
#endif /* ADK_MCU_FREQ != 20000000 */

#ifdef AVR_USB_DEBUG

#ifndef __ASSEMBLER__

/** Set debug token. The token is 4-bits integer which is output to the
 * configured debug port.
 */
#define AVR_USB_DBG_SET(__token) \
    (AVR_USB_DBGPORT_PORT = (AVR_USB_DBGPORT_PORT & 0xf0) | ((__token) & 0x0f))

#else /* __ASSEMBLER__ */

#define AVR_USB_DBG_SET(__token) m_AVR_USB_DBG_SET (__token)

#endif /* __ASSEMBLER__ */

#else /* AVR_USB_DEBUG */

#define AVR_USB_DBG_SET(__token)

#endif /* AVR_USB_DEBUG */

/* USB device states as per fig. 9.1 of the specification. */
#define ADK_USB_STATE_POWERED       0
#define ADK_USB_STATE_DEFAULT       1
#define ADK_USB_STATE_ADDRESS       2
#define ADK_USB_STATE_CONFIGURED    3
#define ADK_USB_STATE_SUSPENDED     4
/** Mask to get state from @ref adkUsbState. */
#define ADK_USB_STATE_MASK          0x7
/** Indicates which receiving buffer is active (in @ref adkUsbState). */
#define ADK_USB_F_CUR_RX_BUF        0x8

/** Maximal allowed data payload for low-speed devices is 8 bytes. */
#define ADK_USB_MAX_DATA_SIZE       8

/** Receiving buffer size. PID + data payload + CRC16. */
#define ADK_USB_RX_BUF_SIZE         (3 + ADK_USB_MAX_DATA_SIZE)

#ifndef __ASSEMBLER__

/** Current USB device state and flags. */
extern u8 adkUsbState;
/** Two receiving buffers which are swapped after each received packet with
 * payload (i.e. which must be processed by user code). Token packets,
 * handshakes and packets to other functions are processed immediately.
 */
extern u8 adkUsbRxBuf[];
/** Number of bytes in current shadow RX buffer available to read by user code. */
extern u8 adkUsbRxSize;

/** Prepare USB interface. */
void
AdkUsbSetup();

/** USB interrupt handler. The application should call this function when it
 * detects data line level change (data lines idle state is low D+ and high D-
 * level). It is up to application to set up interrupt and provide ISR which
 * will call this function. So any custom hardware mechanism can be used for
 * interrupt generation (e.g. separate circuit and port pin). Delay for ISR
 * prologue and function call is acceptable since this function will synchronize
 * with SYNC pattern which precedes each packet.
 * Keep in mind that all packet reception and response is handled inside this
 * function with interrupts globally disabled. This can consume a lot of CPU
 * time when large USB traffic is processed (even when destination is another
 * USB function).
 * It is up to user defined fuction to reset pending interrupt flag if required
 * in order to not produce false interrupt.
 */
void
AdkUsbInterrupt();

/** This function should be called in the application main loop.
 * XXX functionality
 */
void
AdkUsbPoll();

/** Calculate CRC-16-ANSI checksum for the specified data.
 *
 * @param data Pointer to the buffer with data.
 * @param len Length in bytes of the data provided.
 * @return CRC-16 value.
 */
u16
AdkUsbCrc16(u8 *data, u8 len);

#endif /* __ASSEMBLER__ */

#endif /* USB_H_ */
