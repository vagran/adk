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
u8 *adkUsbSysTxData;
u8 *adkUsbUserTxData;
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
}

void
AdkUsbPoll()
{
    if (adkUsbRxState & ADK_USB_RX_SIZE_MASK) {
        /* Have incoming data. */
        if (adkUsbRxState & ADK_USB_RX_SETUP) {
            /* SETUP data received, process the request. */
            AdkUsbSetupData *req = (AdkUsbSetupData *)AdkUsbGetRxData();
            bool_t hasFailed = FALSE;

            if ((req->bmRequestType & ADK_USB_REQ_TYPE_TYPE_MASK) ==
                ADK_USB_REQ_TYPE_TYPE_STANDARD) {
                /* Standard request received. */
                if (req->bRequest == ADK_USB_REQ_SET_ADDRESS) {
                    /* Device address designated by a host. Assuming it is
                     * correct (1-127), no resources to check.
                     */
                    adkUsbNewDeviceAddress = req->wValue;
                } else if (req->bRequest == ADK_USB_REQ_GET_DESCRIPTOR) {

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

            /* State should be modified atomically. */
            u8 nextState;
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
            cli();
            adkUsbRxState &= ~(ADK_USB_RX_SETUP | ADK_USB_RX_SIZE_MASK);
            adkUsbState = (adkUsbState & ~ADK_USB_STATE_MASK) | nextState |
                (hasFailed ? ADK_USB_STATE_TRANS_FAILED : 0);
            sei();
        }
    }
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
