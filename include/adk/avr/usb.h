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

/* PID values with inverted check fields, as read from packet first byte. */
#define ADK_USB_PID_OUT             0xe1
#define ADK_USB_PID_IN              0x69
#define ADK_USB_PID_SETUP           0x2d
#define ADK_USB_PID_DATA0           0xc3
#define ADK_USB_PID_DATA1           0x4b
#define ADK_USB_PID_ACK             0xd2
#define ADK_USB_PID_NAK             0x5a
#define ADK_USB_PID_STALL           0x1e

/** SYNC pattern as it is transferred. */
#define ADK_USB_SYNC_PAT            0x80

/* USB device states as per diagram in doc/pages/product/avr/usb.txt. */
#define ADK_USB_STATE_POWERED           0
#define ADK_USB_STATE_LISTEN            1
#define ADK_USB_STATE_SETUP             2
#define ADK_USB_STATE_WRITE_DATA        3
#define ADK_USB_STATE_WRITE_STATUS      4
#define ADK_USB_STATE_READ_DATA         5
#define ADK_USB_STATE_READ_STATUS       6
/** Mask to get state from @ref adkUsbState. */
#define ADK_USB_STATE_MASK              0x7
#define ADK_USB_STATE_TRANS_ACTIVE_BIT  3
/** Transaction still is not complete. */
#define ADK_USB_STATE_TRANS_ACTIVE      _BV(ADK_USB_STATE_TRANS_ACTIVE_BIT)
#define ADK_USB_STATE_TRANS_FAILED_BIT  4
/** Error occurred in the last transaction. */
#define ADK_USB_STATE_TRANS_FAILED      _BV(ADK_USB_STATE_TRANS_FAILED_BIT)

/** Mask for size field in @ref adkUsbRxState. Non-zero field value indicates
 * number of bytes received in data stage and pending for processing in
 * non-active receiving buffer. The number does not include PID and CRC bytes
 * however they are also always present in the buffer. Zero value indicates that
 * there are no data pending.
 */
#define ADK_USB_RX_SIZE_MASK        0xf
#define ADK_USB_RX_CUR_BUF_BIT      4
/** Indicates which receiving buffer is active (in @ref adkUsbRxState). */
#define ADK_USB_RX_CUR_BUF          _BV(ADK_USB_RX_CUR_BUF_BIT)
#define ADK_USB_RX_MINE_BIT         5
/** Indicates that current transactions packets addressed to this device (set
 * after token decoding and address checking, reset after transaction completion).
 */
#define ADK_USB_RX_MINE             _BV(ADK_USB_RX_MINE_BIT)
#define ADK_USB_RX_SETUP_BIT        6
/** Set when setup request is received, not data. */
#define ADK_USB_RX_SETUP            _BV(ADK_USB_RX_SETUP_BIT)

/** Maximal allowed data payload for low-speed devices is 8 bytes. */
#define ADK_USB_MAX_DATA_SIZE       8

/** Receiving buffer size. PID + data payload + CRC16. */
#define ADK_USB_RX_BUF_SIZE         (3 + ADK_USB_MAX_DATA_SIZE)
/** Transmission buffer size for data packets. SYNC + PID + data + CRC16. */
#define ADK_USB_TX_BUF_SIZE         (4 + ADK_USB_MAX_DATA_SIZE)
/** Transmission buffer size for handshake packets. Used to generate instant
 * responses (e.g. ACK) in transceiver module. Also should be able to hold
 * empty data packets in status stage. SYNC + PID + CRC16.
 */
#define ADK_USB_TX_AUX_BUF_SIZE     4
/** When this flag is set in @ref adkTxDataSize the corresponding data pointer in
 * @ref adkUsbSysTxData or @ref adkUsbSysTxData points to program memory.
 */
#define ADK_USB_TX_PROGMEM_PTR      0x80
/** Maximal amount of data which can be transmitted in one transaction (multiple
 * packets).
 */
#define ADK_USB_TX_MAX_SIZE         0x7f

/** Mask for size field in @ref adkUsbTxState. Non-zero value indicates that
 * data (including SYNC and CRC) placed in the buffer and ready for transmission.
 */
#define ADK_USB_TX_SIZE_MASK        0xf
/** Flag indicates that currently system data are being transmitted (using @ref
 * adkUsbSysTxData pointer).
 */
#define ADK_USB_TX_SYS              0x10
/** Constant which is returned by FetchPacket() to indicate that no data will be
 * transmitted. When adding to packet overhead (SYNC + PID + CRC) it results to
 * zero.
 */
#define ADK_USB_TX_NO_DATA          ((u8)-4)

#ifndef __ASSEMBLER__

/** Setup transaction data payload format (Table 9-2 of the specification). */
typedef struct {
    /** Characteristics of the request (direction, type, recipient). */
    u8 bmRequestType;
    /** Specific request. */
    u8 bRequest;
    /** Word-sized field that varies according to request. */
    u16 wValue;
    /** Word-sized field that varies according to request; typically used to
     * pass an index or offset.
     */
    u16 wIndex;
    /** Number of bytes to transfer if there is a Data stage. */
    u16 wLength;
} AdkUsbSetupData;

/* Bits in bmRequestType field of AdkUsbSetupData. */

/** Data transfer direction. */
#define ADK_USB_REQ_TYPE_DIR_MASK       0x80
/** Host-to-device. */
#define ADK_USB_REQ_TYPE_DIR_H2D        0x00
/** Device-to-host. */
#define ADK_USB_REQ_TYPE_DIR_D2H        0x80
/** Type. */
#define ADK_USB_REQ_TYPE_TYPE_MASK      0x60
#define ADK_USB_REQ_TYPE_TYPE_STANDARD  0x00
#define ADK_USB_REQ_TYPE_TYPE_CLASS     0x20
#define ADK_USB_REQ_TYPE_TYPE_VENDOR    0x40
/** Recipient. */
#define ADK_USB_REQ_TYPE_RCP_MASK       0x1f
/** Device. */
#define ADK_USB_REQ_TYPE_RCP_DEV        0x00
/** Interface. */
#define ADK_USB_REQ_TYPE_RCP_IF         0x01
/** Endpoint. */
#define ADK_USB_REQ_TYPE_RCP_EP         0x02

/* Values for bRequest field of AdkUsbSetupData (Table 9-4). */
#define ADK_USB_REQ_GET_STATUS          0x00
#define ADK_USB_REQ_CLEAR_FEATURE       0x01
#define ADK_USB_REQ_SET_FEATURE         0x03
#define ADK_USB_REQ_SET_ADDRESS         0x05
#define ADK_USB_REQ_GET_DESCRIPTOR      0x06
#define ADK_USB_REQ_SET_DESCRIPTOR      0x07
#define ADK_USB_REQ_GET_CONFIGURATION   0x08
#define ADK_USB_REQ_SET_CONFIGURATION   0x09
#define ADK_USB_REQ_GET_INTERFACE       0x0a
#define ADK_USB_REQ_SET_INTERFACE       0x0b
#define ADK_USB_REQ_SYNC_FRAME          0x0c
/** Custom request for transferring data from host to device. */
#define ADK_USB_REQ_ADK_WRITE           0xf0
/** Custom request for transferring data from device to host. */
#define ADK_USB_REQ_ADK_READ            0xf1

/* Descriptor types used in GET/SET_DESCRIPTOR requests (Table 9-5). */
#define ADK_USB_DESC_TYPE_DEVICE        0x01
#define ADK_USB_DESC_TYPE_CONFIGURATION 0x02
#define ADK_USB_DESC_TYPE_STRING        0x03
#define ADK_USB_DESC_TYPE_INTERFACE     0x04
#define ADK_USB_DESC_TYPE_ENDPOINT      0x05

/** String index for manufacturer name. */
#define ADK_USB_STRING_IDX_MANUFACTURER 1
/** String index for product name. */
#define ADK_USB_STRING_IDX_PRODUCT      2
/** String index for serial number. */
#define ADK_USB_STRING_IDX_SERIAL       3

/** Standard device descriptor (Table 9-8). */
typedef struct {
    /** Size of this descriptor in bytes. */
    u8 bLength;
    /** Device descriptor type. */
    u8 bDescriptorType;
    /** USB Specification Release Number in Binary-Coded Decimal (i.e., 2.10 is
     * 210H). This field identifies the release of the USB Specification with
     * which the device and its descriptors are compliant.
     */
    u16 bcdUSB;
    /** Class code (assigned by the USB-IF). If this field is reset to zero,
     * each interface within a configuration specifies its own class information
     * and the various interfaces operate independently. If this field is set to
     * a value between 1 and FEH, the device supports different class
     * specifications on different interfaces and the interfaces may not operate
     * independently. This value identifies the class definition used for the
     * aggregate interfaces. If this field is set to FFH, the device class is
     * vendor-specific.
     */
    u8 bDeviceClass;
    /** Subclass code (assigned by the USB-IF). These codes are qualified by the
     * value of the bDeviceClass field. If the bDeviceClass field is reset to
     * zero, this field must also be reset to zero. If the bDeviceClass field is
     * not set to FFH, all values are reserved for assignment by the USB-IF.
     */
    u8 bDeviceSubClass;
    /** Protocol code (assigned by the USB-IF). These codes are qualified by the
     * value of the bDeviceClass and the bDeviceSubClass fields. If a device
     * supports class-specific protocols on a device basis as opposed to an
     * interface basis, this code identifies the protocols that the device uses
     * as defined by the specification of the device class. If this field is
     * reset to zero, the device does not use class-specific protocols on a
     * device basis. However, it may use class-specific protocols on an
     * interface basis. If this field is set to FFH, the device uses a vendor-
     * specific protocol on a device basis.
     */
    u8 bDeviceProtocol;
    /** Maximum packet size for endpoint zero (only 8, 16, 32, or 64 are valid). */
    u8 bMaxPacketSize0;
    /** Vendor ID (assigned by the USB-IF). */
    u16 idVendor;
    /** Product ID (assigned by the manufacturer). */
    u16 idProduct;
    /** Device release number in binary-coded decimal. */
    u16 bcdDevice;
    /** Index of string descriptor describing manufacturer. */
    u8 iManufacturer;
    /** Index of string descriptor describing product. */
    u8 iProduct;
    /** Index of string descriptor describing the device’s serial number. */
    u8 iSerialNumber;
    /** Number of possible configurations. */
    u8 bNumConfigurations;
} AdkUsbDeviceDesc;

/** Standard configuration descriptor (Table 9-10). */
typedef struct {
    /** Size of this descriptor in bytes. */
    u8 bLength;
    /** Configuration descriptor type. */
    u8 bDescriptorType;
    /** Total length of data returned for this configuration. Includes the
     * combined length of all descriptors (configuration, interface, endpoint,
     * and class- or vendor-specific) returned for this configuration.
     */
    u16 wTotalLength;
    /** Number of interfaces supported by this configuration. */
    u8 bNumInterfaces;
    /** Value to use as an argument to the SetConfiguration() request to select
     * this configuration.
     */
    u8 bConfigurationValue;
    /** Index of string descriptor describing this configuration. */
    u8 iConfiguration;
    /** Configuration characteristics. */
    u8 bmAttributes;
    /** Maximum power consumption of the USB device from the bus in this
     * specific configuration when the device is fully operational. Expressed in
     * 2 mA units (i.e., 50 = 100 mA).
    */
    u8 bMaxPower;
} AdkUsbConfigDesc;

/* Values for bmAttributes field of AdkUsbConfigDesc. */
/** D7 is reserved and must be set to one for historical reasons. */
#define ADK_USB_CONF_ATTR_ONE           0x80
/** A device configuration that uses power from the bus and a local source
 * reports a non-zero value in bMaxPower to indicate the amount of bus power
 * required and sets D6. The actual power source at runtime may be determined
 * using the GetStatus(DEVICE) request (see Section 9.4.5).
 */
#define ADK_UBS_CONF_ATTR_SELF_POWERED  0x40
/** Set if a device configuration supports remote wake-up. */
#define ADK_UBS_CONF_ATTR_REMOTE_WAKEUP 0x20

/** Current USB device state and flags. */
extern u8 adkUsbState;
/** Two receiving buffers which are swapped after each received packet with
 * payload (i.e. which must be processed by user code). Token packets,
 * handshakes and packets to other functions are processed immediately. However
 * they need buffer space to be received. So if we have one buffer occupied by
 * received DATA packet the second one can be used only for processing tokens
 * and handshakes. If the second data packet is received before the first is
 * completely processed by the application, NAK is issued.
 */
extern u8 adkUsbRxBuf[];
/** Actually is a bit-field variable reflecting receiver state. */
extern u8 adkUsbRxState;
/** Currently assigned device address. Zero if the device is below ADDRESS state. */
extern u8 adkUsbDeviceAddress;
/** Currently pending device address if non-zero. Should be applied only when
 * the SET_ADDRESS request transaction completes (final ACK sent).
 */
extern u8 adkUsbNewDeviceAddress;
/** Transmitter state. */
extern u8 adkUsbTxState;
/** Transmission buffer for data packets. */
extern u8 adkUsbTxDataBuf[];
/** Transmission buffer for handshake packets. */
extern u8 adkUsbTxAuxBuf[];

/** Pointer to transmission data. */
typedef union {
    /** Pointer as integer value. */
    uintptr_t ui_ptr;
    /** Pointer to data in RAM. */
    u8 *ram_ptr;
    /** Pointer in program memory. */
    PGM_P pgm_ptr;
} AdkUsbTxDataPtr;

/** Pointer to pending outgoing system data (e.g. descriptors). */
extern AdkUsbTxDataPtr adkUsbSysTxData;
/** Pointer to pending outgoing user data. */
extern AdkUsbTxDataPtr adkUsbUserTxData;

/** Size of data pointed by @ref adkUsbSysTxData or @ref adkUsbUserTxData.
 * Most significant bit indicates that data are located in program memory (see
 * @ref ADK_USB_TX_PROGMEM_PTR.
 */
extern u8 adkTxDataSize;
/** Device descriptor. */
extern const PROGMEM AdkUsbDeviceDesc adkUsbDeviceDesc;

/** Get data in shadow receiving buffer (after PID). */
static inline u8 *
AdkUsbGetRxData()
{
    return adkUsbRxState & ADK_USB_RX_CUR_BUF ?
                adkUsbRxBuf + 1:
                adkUsbRxBuf + ADK_USB_RX_BUF_SIZE + 1;
}

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
 * It is up to user defined function to reset pending interrupt flag if required
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
