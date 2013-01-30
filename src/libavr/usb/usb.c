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
u8 adkUsbRxPrevDataID;

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
    0x200,
    /* bDeviceClass */
    ADK_USB_DEVICE_CLASS,
    /* bDeviceSubClass */
    ADK_USB_DEVICE_SUBCLASS,
    /* bDeviceProtocol */
    0xff,
    /* bMaxPacketSize0 */
    ADK_USB_MAX_DATA_SIZE,
    /* idVendor */
    ADK_USB_VENDOR_ID,
    /* idProduct */
    ADK_USB_PRODUCT_ID,
    /* bcdDevice */
    ADK_USB_VERSION,

    /* iManufacturer */
#   ifdef ADK_USB_MANUFACTURER_STRING
    ADK_USB_STRING_IDX_MANUFACTURER,
#   else
    0,
#   endif

    /* iProduct */
#   ifdef ADK_USB_PRODUCT_STRING
    ADK_USB_STRING_IDX_PRODUCT,
#   else
    0,
#   endif

    /* iSerialNumber */
#   ifdef ADK_USB_SERIAL_STRING
    ADK_USB_STRING_IDX_SERIAL,
#   else
    0,
#   endif

    /* bNumConfigurations */
    1
};

const PROGMEM AdkUsbFullConfigDesc adkUsbConfigDesc = {
    /* Configuration descriptor. */
    {
        /* bLength */
        sizeof(AdkUsbConfigDesc),
        /* bDescriptorType */
        ADK_USB_DESC_TYPE_CONFIGURATION,
        /* wTotalLength */
        sizeof(AdkUsbFullConfigDesc),
        /* bNumInterfaces */
        1,
        /* bConfigurationValue */
        1,
        /* iConfiguration */
        0,
        /* bmAttributes */
        ADK_USB_CONF_ATTR_ONE,
        /* bMaxPower */
        ADK_USB_POWER_CONSUMPTION / 2
    },
    /* Interface descriptor. */
    {
        /* bLength */
        sizeof(AdkUsbInterfaceDesc),
        /* bDescriptorType */
        ADK_USB_DESC_TYPE_INTERFACE,
        /* bInterfaceNumber */
        0,
        /* bAlternateSetting */
        0,
        /* bNumEndpoints */
        0,
        /* bInterfaceClass */
        0xff,
        /* bInterfaceSubClass */
        0xff,
        /* bInterfaceProtocol */
        0xff,
        /* iInterface */
        0
    }
};

/* Strings are initialized without terminating null. */
const PROGMEM AdkUsbFullStringDesc adkUsbFullStringDesc = {
    /* Languages array. */
    {
        /* Header */
        {
            /* bLength */
            sizeof(adkUsbFullStringDesc.lang),
            /* bDescriptorType */
            ADK_USB_DESC_TYPE_STRING
        },
        /* wLANGID */
        ADK_USB_LANGID_US_ENGLISH
    },

    /* Manufacturer string. */
#   ifdef ADK_USB_MANUFACTURER_STRING
    {
        /* Header */
        {
            /* bLength */
            sizeof(adkUsbFullStringDesc.manufacturer),
            /* bDescriptorType */
            ADK_USB_DESC_TYPE_STRING
        },
        /* string */
        ADK_USB_STRING(ADK_USB_MANUFACTURER_STRING)
    },
#   endif /* ADK_USB_MANUFACTURER_STRING */

    /* Product string. */
#   ifdef ADK_USB_PRODUCT_STRING
    {
        /* Header */
        {
            /* bLength */
            sizeof(adkUsbFullStringDesc.product),
            /* bDescriptorType */
            ADK_USB_DESC_TYPE_STRING
        },
        /* string */
        ADK_USB_STRING(ADK_USB_PRODUCT_STRING)
    },
#   endif /* ADK_USB_PRODUCT_STRING */

    /* Serial string. */
#   ifdef ADK_USB_SERIAL_STRING
    {
        /* Header */
        {
            /* bLength */
            sizeof(adkUsbFullStringDesc.serial),
            /* bDescriptorType */
            ADK_USB_DESC_TYPE_STRING
        },
        /* string */
        ADK_USB_STRING(ADK_USB_SERIAL_STRING)
    }
#   endif /* ADK_USB_SERIAL_STRING */
};

void
AdkUsbSetup()
{
    /* Configure debug port if enabled. */
#   ifdef AVR_USB_DEBUG
    ADK_USB_DBGPORT_DDR |= 0x0f;
    ADK_USB_DBGPORT_PORT &= 0xf0;
#   endif /* AVR_USB_DEBUG */

    /* Configure data lines for input and disable pull-up resistors. */
    AVR_BIT_CLR8(ADK_USB_DPORT_DDR, ADK_USB_DPLUS_PIN);
    AVR_BIT_CLR8(ADK_USB_DPORT_DDR, ADK_USB_DMINUS_PIN);
    AVR_BIT_CLR8(ADK_USB_DPORT_PORT, ADK_USB_DPLUS_PIN);
    AVR_BIT_CLR8(ADK_USB_DPORT_PORT, ADK_USB_DMINUS_PIN);

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
 * initialized to previous DATAX ID (will be toggled in this function). The
 * function can leave the non-zero pointer for the last chunk which anyway
 * will be overwritten in the start of the next transaction.
 * @return Full size of data prepared in transmission buffer.
 */
static u8
FetchPacket()
{
    AdkUsbTxDataPtr *pptr, ptr;

    if (adkUsbTxState & ADK_USB_TX_SYS) {
        pptr = &adkUsbSysTxData;
    } else {
        pptr = &adkUsbUserTxData;
    }
    ptr.ui_ptr = pptr->ui_ptr;

    if (!ptr.ui_ptr) {
        return 0;
    }

    /* Fetch not more than ADK_USB_MAX_DATA_SIZE at once. If data size is
     * multiple of the granularity, leave the pointer non-zero to send zero-sized
     * packet at the end.
     */
    u8 size = adkTxDataSize & ~ADK_USB_TX_PROGMEM_PTR;
    if (size > ADK_USB_MAX_DATA_SIZE) {
        size = ADK_USB_MAX_DATA_SIZE;
    }

    /* Toggle DATAX ID. */
    adkUsbTxDataBuf[1] ^= ADK_USB_PID_DATA0 ^ ADK_USB_PID_DATA1;

    /* Copy to transmission buffer skipping SYNC and PID. */
    if (adkTxDataSize & ADK_USB_TX_PROGMEM_PTR) {
        /* Data in program memory. */
        memcpy_P(&adkUsbTxDataBuf[2], ptr.pgm_ptr, size);
    } else {
        /* Data in RAM. */
        memcpy(&adkUsbTxDataBuf[2], ptr.ram_ptr, size);
    }

    adkTxDataSize -= size;
    pptr->ui_ptr += size;

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
    /* Request processing failed if ADK_USB_STATE_TRANS_FAILED. */
    u8 hasFailed = 0;
    /* Next state if non-zero. */
    u8 nextState = 0;

    u8 rxSize = adkUsbRxState & ADK_USB_RX_SIZE_MASK;
    if (rxSize) {
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
                    adkUsbNewDeviceAddress = req->wValue.bytes[0];

                } else if (req->bRequest == ADK_USB_REQ_GET_DESCRIPTOR) {
                    /* Check requested descriptor type. */
                    u8 size;
                    u8 descType = req->wValue.bytes[1];

                    if (descType == ADK_USB_DESC_TYPE_DEVICE) {
                        adkUsbSysTxData.pgm_ptr = (PGM_P)&adkUsbDeviceDesc;
                        size = sizeof(adkUsbDeviceDesc);

                    } else if (descType == ADK_USB_DESC_TYPE_CONFIGURATION) {
                        adkUsbSysTxData.pgm_ptr = (PGM_P)&adkUsbConfigDesc;
                        size = sizeof(AdkUsbFullConfigDesc);
                    } else if (descType == ADK_USB_DESC_TYPE_STRING) {
                        descType = req->wValue.bytes[0];

                        if (descType == ADK_USB_STRING_IDX_LANG) {
                            adkUsbSysTxData.pgm_ptr = (PGM_P)&adkUsbFullStringDesc.lang;
                            size = sizeof(adkUsbFullStringDesc.lang);

#                       ifdef ADK_USB_MANUFACTURER_STRING
                        } else if (descType == ADK_USB_STRING_IDX_MANUFACTURER) {
                            adkUsbSysTxData.pgm_ptr = (PGM_P)&adkUsbFullStringDesc.manufacturer;
                            size = sizeof(adkUsbFullStringDesc.manufacturer);
#                       endif /* ADK_USB_MANUFACTURER_STRING */

#                       ifdef ADK_USB_PRODUCT_STRING
                        } else if (descType == ADK_USB_STRING_IDX_PRODUCT) {
                            adkUsbSysTxData.pgm_ptr = (PGM_P)&adkUsbFullStringDesc.product;
                            size = sizeof(adkUsbFullStringDesc.product);
#                       endif /* ADK_USB_PRODUCT_STRING */

#                       ifdef ADK_USB_SERIAL_STRING
                        } else if (descType == ADK_USB_STRING_IDX_SERIAL) {
                            adkUsbSysTxData.pgm_ptr = (PGM_P)&adkUsbFullStringDesc.serial;
                            size = sizeof(adkUsbFullStringDesc.serial);
#                       endif /* ADK_USB_SERIAL_STRING */

                        } else {
                            hasFailed = ADK_USB_STATE_TRANS_FAILED;
                        }

                    } else {
                        hasFailed = ADK_USB_STATE_TRANS_FAILED;
                    }
                    if (!hasFailed) {
                        adkUsbTxState |= ADK_USB_TX_SYS;
                        adkTxDataSize = MIN(size, (u8)req->wLength) | ADK_USB_TX_PROGMEM_PTR;
                        /* PID will be toggled in FetchPacket(). */
                        adkUsbTxDataBuf[1] = ADK_USB_PID_DATA0;
                    }

                } else if (req->bRequest != ADK_USB_REQ_SET_CONFIGURATION) {
                    /* Configuration request is ignored, all the rest cause error. */
                    hasFailed = ADK_USB_STATE_TRANS_FAILED;
                }
            } else if ((req->bmRequestType & ADK_USB_REQ_TYPE_TYPE_MASK) ==
                       ADK_USB_REQ_TYPE_TYPE_VENDOR) {
                /* Vendor-specific request, most probably ADK I/O. */
                if (req->bRequest == ADK_USB_REQ_ADK_READ) {
                    adkUsbTxState &= ~ADK_USB_TX_SYS;
                    adkTxDataSize = AdkUsbOnTransmit(req->wLength);
                    /* PID will be toggled in FetchPacket(). */
                    adkUsbTxDataBuf[1] = ADK_USB_PID_DATA0;
                } else if (req->bRequest != ADK_USB_REQ_ADK_WRITE) {
                    hasFailed = ADK_USB_STATE_TRANS_FAILED;
                }
            } else {
                hasFailed = ADK_USB_STATE_TRANS_FAILED;
            }

            if ((req->bmRequestType & ADK_USB_REQ_TYPE_DIR_MASK) ==
                ADK_USB_REQ_TYPE_DIR_H2D) {

                /* Write request. */
                if (req->wLength) {
                    nextState = ADK_USB_STATE_WRITE_DATA;
                    /* Initialize data PID toggling control. Write transfer
                     * should start from DATA1 PID so set previous to DATA0.
                     */
                    adkUsbRxPrevDataID = ADK_USB_PID_DATA0;
                } else {
                    nextState = ADK_USB_STATE_WRITE_STATUS;
                }
            } else {
                /* Read request. */
                nextState = ADK_USB_STATE_READ_DATA;
            }
        } else {
            /* Any payload of write requests is interpreted as user data. All
             * system write requests which are supported have all data in the
             * setup payload so it will be handled above. Hold the receiving
             * buffer if the client callback returns FALSE.
             */
            if (!AdkUsbOnReceive(AdkUsbGetRxData(), rxSize)) {
                rxSize = 0;
            }
        }
    }

    /* Fetch next out-coming packet if ISR is waiting for TX data. It is safe to
     * check this flag here because it is not reset until some packet is
     * transmitted. Packet less than ADK_USB_MAX_DATA_SIZE terminates data stage
     * of the read transaction.
     */
    /* Size of prepared transmission data. */
    u8 txSize;
    if (adkUsbState & ADK_USB_STATE_READ_WAIT) {
        txSize = FetchPacket();
    } else {
        txSize = 0;
    }

    /* State should be modified atomically. */
    cli();
    if (rxSize) {
        /* Free RX buffer. */
        adkUsbRxState &= ~(ADK_USB_RX_SETUP | ADK_USB_RX_SIZE_MASK);
    }
    register u8 state = adkUsbState | hasFailed;
    if (nextState) {
        state = (state & ~ADK_USB_STATE_MASK) | nextState;
    }
    /* Pass TX buffer if any data ready. */
    if (txSize) {
        adkUsbTxState = (adkUsbTxState & ~ADK_USB_TX_SIZE_MASK) | txSize;
        state &= ~ADK_USB_STATE_READ_WAIT;
    }
    adkUsbState = state;
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
    adkTxDataSize = 0;
    adkUsbTxState = 0;
}
