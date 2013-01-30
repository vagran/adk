/* /ADK/samples/avr/client/main.cpp
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file main.cpp
 * Sample AVR USB desktop client.
 */

#include <adk.h>
#include <app_config.h>

int
main(int argc, char **argv)
{
    ADK_INFO("AVR USB client sample");
    adk::LibusbCtx ctx;
    auto device = ctx.OpenDeviceByPid(ADK_USB_VENDOR_ID, ADK_USB_PRODUCT_ID);
    if (!device) {
        ADK_WARNING("Cannot open device");
        return 1;
    }
    ADK_INFO("Device address: %d", device->GetAddress());

    u8 buf[128] = {'\x08'};
    size_t size = device->Write(buf, 3);//XXX
    ADK_INFO("%lu bytes written", size);
    memset(buf, 0, sizeof(buf));
    size = device->Read(buf, 3);
    ADK_INFO("%lu bytes read", size);
    for (size_t i = 0; i < size; i++) {
        ADK_INFO("%02lx: %02x", i, buf[i]);
    }

    return 0;
}
