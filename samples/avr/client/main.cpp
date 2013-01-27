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
    auto device = ctx.OpenDeviceByPid(AVR_USB_VENDOR_ID, AVR_USB_PRODUCT_ID);
    if (!device) {
        ADK_WARNING("Cannot open device");
        return 1;
    }
    ADK_INFO("Device address: %d", device->GetAddress());
    device->Write("aaa", 3);//XXX
    return 0;
}
