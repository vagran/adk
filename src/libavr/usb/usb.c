/* /ADK/src/libavr/usb/usb.c
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file usb.c
 * USB interface software implementation.
 */

#include <adk.h>

u8 adkUsbState;
u8 adkUsbTxState;
u8 adkUsbRxState;

u8 adkUsbRxBuf[2 * ADK_USB_RX_BUF_SIZE];
u8 adkUsbTxDataBuf[ADK_USB_TX_BUF_SIZE];
u8 adkUsbTxAuxBuf[ADK_USB_TX_AUX_BUF_SIZE];

u8 adkUsbDeviceAddress;
u8 adkUsbNewDeviceAddress;
AdkUsbTxDataPtr adkUsbSysTxData;
AdkUsbTxDataPtr adkUsbUserTxData;
u8 adkTxDataSize;

const PROGMEM AdkUsbDeviceDesc adkUsbDeviceDesc = {
    /* bLength */
    sizeof(AdkUsbDeviceDesc),
    /* bDescriptorType */
    ADK_USB_DESC_TYPE_DEVICE,
    /* bcdUSB */
    0x210,
    /* bDeviceClass */
    AVR_USB_DEVICE_CLASS,
    /* bDeviceSubClass */
    AVR_USB_DEVICE_SUBCLASS,
    /* bDeviceProtocol */
    0xff,
    /* bMaxPacketSize0 */
    ADK_USB_MAX_DATA_SIZE,
    /* idVendor */
    AVR_USB_VENDOR_ID,
    /* idProduct */
    AVR_USB_PRODUCT_ID,
    /* bcdDevice */
    AVR_USB_VERSION,
    /* iManufacturer */
    AVR_USB_MANUFACTURER_STRING != 0 ? ADK_USB_STRING_IDX_MANUFACTURER : 0,
    /* iProduct */
    AVR_USB_PRODUCT_STRING != 0 ? ADK_USB_STRING_IDX_PRODUCT : 0,
    /* iSerialNumber */
    AVR_USB_SERIAL_STRING != 0 ? ADK_USB_STRING_IDX_SERIAL : 0,
    /* bNumConfigurations */
    1
};

void
AdkUsbSetup()
{
    /* Configure debug port if enabled. */
#   ifdef AVR_USB_DEBUG
    AVR_USB_DBGPORT_DDR |= 0x0f;
    AVR_USB_DBGPORT_PORT &= 0xf0;
#   endif /* AVR_USB_DEBUG */

    /* Configure data lines for input and disable pull-up resistors. */
    AVR_BIT_CLR8(AVR_USB_DPORT_DDR, AVR_USB_DPLUS_PIN);
    AVR_BIT_CLR8(AVR_USB_DPORT_DDR, AVR_USB_DMINUS_PIN);
    AVR_BIT_CLR8(AVR_USB_DPORT_PORT, AVR_USB_DPLUS_PIN);
    AVR_BIT_CLR8(AVR_USB_DPORT_PORT, AVR_USB_DMINUS_PIN);

    adkUsbState = 0;
    adkUsbDeviceAddress = 0;
    adkUsbRxState = 0;

    adkUsbTxAuxBuf[0] = ADK_USB_SYNC_PAT;
    /* CRC-16 for empty data packets. */
    adkUsbTxAuxBuf[2] = 0;
    adkUsbTxAuxBuf[3] = 0;
    adkUsbTxDataBuf[0] = ADK_USB_SYNC_PAT;
}

/** Fetch next packet from outgoing data stream. PID should already be properly
 * initialized.
 * @return Size of data prepared in transmission buffer.
 */
static u8
FetchPacket()
{
    AdkUsbTxDataPtr ptr;
    if (adkUsbTxState & ADK_USB_TX_SYS) {
        ptr.ui_ptr = adkUsbSysTxData.ui_ptr;
    } else {
        ptr.ui_ptr = adkUsbUserTxData.ui_ptr;
    }
    if (!ptr.ui_ptr) {
        return ADK_USB_TX_NO_DATA;
    }
    /* Fetch not more than ADK_USB_MAX_DATA_SIZE at once. If data size is
     * multiple of the granularity, leave the pointer non-zero to send zero-sized
     * packet at the end.
     */
    u8 size = adkTxDataSize & ~ADK_USB_TX_PROGMEM_PTR;
    if (size > ADK_USB_MAX_DATA_SIZE) {
        size = ADK_USB_MAX_DATA_SIZE;
    }
    /* Copy to transmission buffer skipping SYNC and PID. */
    if (adkTxDataSize & ADK_USB_TX_PROGMEM_PTR) {
        /* Data in program memory. */
        memcpy_P(&adkUsbTxDataBuf[2], ptr.pgm_ptr, size);
    } else {
        /* Data in RAM. */
        memcpy(&adkUsbTxDataBuf[2], ptr.ram_ptr, size);
    }
    /* Release outgoing data pointer if the last chunk transmitted. */
    if (size != ADK_USB_MAX_DATA_SIZE) {
        if (adkUsbTxState & ADK_USB_TX_SYS) {
            adkUsbSysTxData.ui_ptr = 0;
            adkUsbTxState &= ~ADK_USB_TX_SYS;
        } else {
            adkUsbUserTxData.ui_ptr = 0;
        }
    }
    /* Calculate CRC. */
    u16 crc = AdkUsbCrc16(adkUsbTxDataBuf + 2, size);
    adkUsbTxDataBuf[size + 2] = AVR_LO8(crc);
    adkUsbTxDataBuf[size + 3] = AVR_HI8(crc);
    /* Total length of the prepared data. */
    return size + 4;
}

void
AdkUsbPoll()
{
    bool_t hasFailed = FALSE;
    u8 nextState = 0;
    u8 txSize = ADK_USB_TX_NO_DATA;

    if (adkUsbRxState & ADK_USB_RX_SIZE_MASK) {
        /* Have incoming data. */
        if (adkUsbRxState & ADK_USB_RX_SETUP) {
            /* SETUP data received, process the request. */
            AdkUsbSetupData *req = (AdkUsbSetupData *)AdkUsbGetRxData();

            if ((req->bmRequestType & ADK_USB_REQ_TYPE_TYPE_MASK) ==
                ADK_USB_REQ_TYPE_TYPE_STANDARD) {
                /* Standard request received. */
                if (req->bRequest == ADK_USB_REQ_SET_ADDRESS) {
                    /* Device address designated by a host. Assuming it is
                     * correct (1-127), no resources to check.
                     */
                    adkUsbNewDeviceAddress = req->wValue;
                } else if (req->bRequest == ADK_USB_REQ_GET_DESCRIPTOR) {
                    adkUsbTxState |= ADK_USB_TX_SYS;
                    adkUsbSysTxData.pgm_ptr = (PGM_P)&adkUsbDeviceDesc;
                    adkTxDataSize = sizeof(adkUsbDeviceDesc) | ADK_USB_TX_PROGMEM_PTR;
                    adkUsbTxDataBuf[1] = ADK_USB_PID_DATA1;
                } else {
                    hasFailed = TRUE;
                }
            } else if ((req->bmRequestType & ADK_USB_REQ_TYPE_TYPE_MASK) ==
                       ADK_USB_REQ_TYPE_TYPE_VENDOR) {
                /* Vendor-specific request, most probably ADK I/O. */
                //XXX
            } else {
                hasFailed = TRUE;
            }

            if ((req->bmRequestType & ADK_USB_REQ_TYPE_DIR_MASK) ==
                ADK_USB_REQ_TYPE_DIR_H2D) {

                /* Write request. */
                if (req->wLength) {
                    nextState = ADK_USB_STATE_WRITE_DATA;
                } else {
                    nextState = ADK_USB_STATE_WRITE_STATUS;
                }
            } else {
                /* Read request. */
                nextState = ADK_USB_STATE_READ_DATA;
            }
        }
    }

    /* Fetch next out-coming packet if read transaction is in progress. */
    if (1) {//XXX
        txSize = FetchPacket();
    }

    /* State should be modified atomically. */
    cli();
    adkUsbRxState &= ~(ADK_USB_RX_SETUP | ADK_USB_RX_SIZE_MASK);
    adkUsbState = (adkUsbState & ~ADK_USB_STATE_MASK) |
        (nextState ? nextState : (adkUsbState & ADK_USB_STATE_MASK)) |
        (hasFailed ? ADK_USB_STATE_TRANS_FAILED : 0);
    adkUsbTxState = (adkUsbTxState & ADK_USB_TX_SIZE_MASK) | (txSize + 4);
    sei();
}

/** This function is called from assembler interrupt handler when reset is
 * detected on the line.
 */
void
_AdkUsbOnReset()
{
    adkUsbState = (adkUsbState & ~ADK_USB_STATE_MASK) | ADK_USB_STATE_LISTEN;
    adkUsbDeviceAddress = 0;
    adkUsbRxState = 0;
    AVR_BIT_SET8(PINB, 3);//XXX
}
