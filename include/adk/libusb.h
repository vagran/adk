/* /ADK/include/adk/libusb.h
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file libusb.h
 * ADK wrapper for libusb.
 */

#ifndef ADK_LIBUSB_H_
#define ADK_LIBUSB_H_

#include <libusb.h>

namespace adk {

/** Parameter for libusb exception which wraps the error code. */
class LibusbExceptionParam {
public:
    LibusbExceptionParam(int code): _code(code) {}

    void
    ToString(std::stringstream &ss) const
    {
        ss << libusb_error_name(_code) << "(" << _code << ")";
    }

    operator int() const
    {
        return _code;
    }

private:
    /** Libusb error code. */
    int _code;
};

/** Libusb exception which is thrown when libusb API returns error code. */
ADK_DEFINE_PARAM_EXCEPTION(LibusbException, LibusbExceptionParam)

/** Throw ADK USB exception.
 * @param code Error code returned by libusb call.
 * @param __msg Message which could be streaming expression. Additional arguments
 *      to the exception class constructor can be specified after the message.
 */
#define ADK_USB_EXCEPTION(__code, __msg) \
    ADK_EXCEPTION(adk::LibusbException, __msg, __code)

class LibusbCtx;

class LibusbDevice {
public:
    typedef std::shared_ptr<LibusbDevice> Handle;

    ~LibusbDevice()
    {
        libusb_close(_hDevice);
    }

    /** Get address assigned to the device. */
    u8
    GetAddress()
    {
        return libusb_get_device_address(_device);
    }

    /** Reset the device. */
    void
    Reset()
    {
        libusb_reset_device(_hDevice);
    }

    /** Write data to device.
     *
     * @param data Data buffer.
     * @param size Size of data in bytes.
     * @param timeout Timeout in milliseconds. Zero fo unlimited timeout.
     * @return Number of bytes actually transferred.
     */
    size_t
    Write(const void *data, size_t size, int timeout = 0)
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

    /** Read data from device.
     * @param data Data buffer.
     * @param size Desired transfer size in bytes.
     * @param timeout Timeout in milliseconds. Zero fo unlimited timeout.
     * @return Number of bytes actually transferred.
     */
    size_t
    Read(void *data, size_t size, int timeout = 0)
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

private:
    friend class LibusbCtx;

    libusb_device_handle *_hDevice;
    libusb_device *_device;

    LibusbDevice(libusb_device_handle *hDevice): _hDevice(hDevice)
    {
        _device = libusb_get_device(hDevice);
    }
};

/** Wrapper class for libusb context. Should be used for all USB operations. */
class LibusbCtx {
public:
    LibusbCtx()
    {
        int ec;
        if ((ec = libusb_init(&_ctx))) {
            ADK_USB_EXCEPTION(ec, "Failed to initialize libusb context");
        }
#       ifdef DEBUG
        libusb_set_debug(_ctx, 3);
#       endif /* DEBUG */
    }

    ~LibusbCtx()
    {
        if (_ctx) {
            libusb_exit(_ctx);
        }
    }

    /** Open device by vendor and product ID.
     * @return Pointer to the device. @a nullptr if opening failed.
     */
    LibusbDevice::Handle
    OpenDeviceByPid(u16 vendorId, u16 productId)
    {
        libusb_device_handle *hDevice =
            libusb_open_device_with_vid_pid(_ctx, vendorId, productId);
        if (!hDevice) {
            return LibusbDevice::Handle(nullptr);
        }
        return LibusbDevice::Handle(new LibusbDevice(hDevice));
    }

private:
    libusb_context *_ctx = nullptr;
};

} /* namespace adk */

#endif /* ADK_LIBUSB_H_ */
