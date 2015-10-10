/* This file is a part of ADK library.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
 */

/** @file libusb.cpp
 * libusb wrapper implementation.
 */

#ifdef ADK_AVR_USE_USB

#include <adk.h>

using namespace adk;

void
LibusbExceptionParam::ToString(std::stringstream &ss) const
{
    ss << libusb_error_name(_code) << "(" << _code << ")";
}

LibusbDevice::~LibusbDevice()
{
    libusb_close(_hDevice);
}

u8
LibusbDevice::GetAddress()
{
    return libusb_get_device_address(_device);
}

void
LibusbDevice::Reset()
{
    libusb_reset_device(_hDevice);
}

size_t
LibusbDevice::Write(const void *data, size_t size, int timeout)
{
    int ec = libusb_control_transfer(_hDevice,
        LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
        ADK_USB_REQ_ADK_WRITE, 0, 0,
        const_cast<u8 *>(static_cast<const u8 *>(data)), size, timeout);
    if (ec < 0) {
        ADK_USB_EXCEPTION(ec, "Failed to write to device");
    }
    return ec;
}

size_t
LibusbDevice::Read(void *data, size_t size, int timeout)
{
    int ec = libusb_control_transfer(_hDevice,
        LIBUSB_RECIPIENT_DEVICE | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
        ADK_USB_REQ_ADK_READ, 0, 0,
        const_cast<u8 *>(static_cast<const u8 *>(data)), size, timeout);
    if (ec < 0) {
        ADK_USB_EXCEPTION(ec, "Failed to read from device");
    }
    return ec;
}

LibusbDevice::LibusbDevice(libusb_device_handle *hDevice):
    _hDevice(hDevice)
{
    _device = libusb_get_device(hDevice);
}

LibusbCtx::LibusbCtx()
{
    int ec;
    if ((ec = libusb_init(&_ctx))) {
        ADK_USB_EXCEPTION(ec, "Failed to initialize libusb context");
    }
#   ifdef DEBUG
    libusb_set_debug(_ctx, 3);
#   endif /* DEBUG */
}

LibusbCtx::~LibusbCtx()
{
    if (_ctx) {
        libusb_exit(_ctx);
    }
}

LibusbDevice::Handle
LibusbCtx::OpenDeviceByPid(u16 vendorId, u16 productId)
{
    libusb_device_handle *hDevice =
        libusb_open_device_with_vid_pid(_ctx, vendorId, productId);
    if (!hDevice) {
        return LibusbDevice::Handle(nullptr);
    }
    return LibusbDevice::Handle(std::make_shared<LibusbDevice>(hDevice));
}

#endif /* ADK_AVR_USE_USB */
