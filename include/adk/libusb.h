/* /ADK/include/adk/libusb.h
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
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

/** Exception class for errors in libusb wrapper. */
class LibusbException: public adk::Exception {
private:
    void
    _AppendErrorCode()
    {
        if (errorCode) {
            std::stringstream ss;
            ss << ": " << libusb_error_name(errorCode) << "(" << errorCode << ")";
            _msg += ss.str();
        }
    }

public:
    int errorCode;

    LibusbException(const char *msg, int code = 0):
        Exception(msg), errorCode(code)
    {
        _AppendErrorCode();
    }

    LibusbException(const std::string &msg, int code = 0):
        Exception(msg), errorCode(code)
    {
        _AppendErrorCode();
    }

#   ifdef DEBUG
    LibusbException(const char *file, int line, const char *msg, int code = 0):
        Exception(file, line, msg), errorCode(code)
    {
        _AppendErrorCode();
    }

    LibusbException(const char *file, int line, const std::string &msg, int code = 0):
        Exception(file, line, msg), errorCode(code)
    {
        _AppendErrorCode();
    }
#   endif /* DEBUG */

    virtual
    ~LibusbException() noexcept
    {}
};

/** Throw ADK USB exception.
 * @param code Error code returned by libusb call.
 * @param __msg Message which could be streaming expression. Additional arguments
 *      to the exception class constructor can be specified after the message.
 */
#define ADK_USB_EXCEPTION(__code, __msg) \
    ADK_EXCEPTION(adk::LibusbException, __msg, __code)

class LibusbCtx;

class LibusbDevice {
private:
    friend class LibusbCtx;

    libusb_device_handle *_hDevice;
    libusb_device *_device;

    LibusbDevice(libusb_device_handle *hDevice): _hDevice(hDevice)
    {
        _device = libusb_get_device(hDevice);
    }

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
};

/** Wrapper class for libusb context. Should be used for all USB operations. */
class LibusbCtx {
private:
    libusb_context *_ctx = nullptr;
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
};

} /* namespace adk */

#endif /* ADK_LIBUSB_H_ */
