/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file main.cpp
 * Sample AVR USB desktop client.
 */

#include <adk.h>
#include <app_config.h>

using namespace adk;

static const size_t BUF_SIZE = 128;

static u8 txBuf[BUF_SIZE], rxBuf[BUF_SIZE];

static int numPatternsTested;

static void
DumpPattern(u8 *buf, size_t size)
{
    ADK_INFO("Pattern %lu bytes:", size);
    for (size_t i = 0; i < size; i++) {
        ADK_INFO("%02lx: %02x", i, buf[i]);
    }
}

static void
TestPattern(LibusbDevice::Handle device, size_t size)
{
    numPatternsTested++;
    try {
        size_t numWritten = device->Write(txBuf, size);
        if (numWritten != size) {
            ADK_EXCEPTION(Exception, "Short write: " << numWritten << "/" << size);
        }
    } catch(LibusbException &) {
        ADK_WARNING("Device writing failed");
        DumpPattern(txBuf, size);
        throw;
    }
    /* Firmware receives data in 8 bytes chunks. Only the remainder chunk is
     * sent back.
     */
    size_t readOffset = (size / 8) * 8;
    size_t readSize = size - readOffset;
    try {
        size_t numRead = device->Read(rxBuf, readSize);
        if (numRead != readSize) {
            ADK_INFO("Received pattern:");
            DumpPattern(rxBuf, numRead);
            ADK_EXCEPTION(Exception, "Short read: " << numRead << "/" << size);
        }
    } catch(LibusbException &) {
        ADK_WARNING("Device reading failed");
        DumpPattern(txBuf, size);
        throw;
    }
    if (memcmp(&txBuf[readOffset], rxBuf, readSize)) {
        ADK_WARNING("Received pattern mismatch");
        ADK_INFO("Transmitted pattern:");
        DumpPattern(txBuf, size);
        ADK_INFO("Received pattern:");
        DumpPattern(rxBuf, readSize);
        ADK_EXCEPTION(Exception, "Pattern invalid echo response");
    }
}

int
main(int argc __UNUSED, char **argv __UNUSED)
{
    ADK_INFO("AVR USB client sample");
    LibusbCtx ctx;
    LibusbDevice::Handle device = ctx.OpenDeviceByPid(ADK_USB_VENDOR_ID, ADK_USB_PRODUCT_ID);
    if (!device) {
        ADK_WARNING("Cannot open device");
        return 1;
    }
    ADK_INFO("Device address: %d", device->GetAddress());

    ADK_INFO("Testing bit patterns with variable length packets...");

    const u8 patterns[] = {
        0x00, 0xff, 0xaa, 0x55, 0x5a, 0xa5, 0x01, 0x02,
        0x04, 0x08, 0x10, 0x20, 0x40, 0x80
    };

    for (u8 pat: patterns) {
        for (size_t len = 1; len <= 32; len++) {
            memset(txBuf, pat, len);
            TestPattern(device, len);
        }
    }

    ADK_INFO("Running bit tests...");
    for (size_t len = 1; len <= 8; len++) {
        for (size_t bitIdx = 0; bitIdx < len * NBBY; bitIdx++) {
            size_t byteIdx = bitIdx / 8;
            size_t bitMask = 1 << (bitIdx - byteIdx * 8);
            /* Running bit one. */
            memset(txBuf, 0, len);
            txBuf[byteIdx] |= bitMask;
            TestPattern(device, len);
            /* Running bit zero. */
            memset(txBuf, 0xff, len);
            txBuf[byteIdx] &= ~bitMask;
            TestPattern(device, len);
        }
    }

    ADK_INFO("Testing one byte packets, all values...");

    for (int i = 0; i < 0x100; i++) {
        txBuf[0] = i;
        TestPattern(device, 1);
    }

    ADK_INFO("Testing two bytes packets, all values...");

    for (int i = 0; i < 0x10000; i++) {
        if (i % 0x400 == 0) {
            ADK_INFO("%04x...", i);
        }
        txBuf[0] = i & 0xff;
        txBuf[1] = i >> 8;
        TestPattern(device, 2);
    }

    ADK_INFO("Test successfully completed, %d patterns tested", numPatternsTested);

    return 0;
}
