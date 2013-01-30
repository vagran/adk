/* /ADK/include/adk/libusb.h
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012, Artyom Lebedev <artyom.lebedev@gmail.com>
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

class LibusbException: public adk::Exception {
public:
    int errorCode;

    LibusbException(int code, const char *msg):
        Exception(msg), errorCode(code)
    {
        if (code) {
            _msg += ": ";
            _msg += libusb_error_name(code);
        }
    }

    LibusbException(int code, const std::string &msg):
        Exception(msg), errorCode(code)
    {
        if (code) {
            _msg += ": ";
            _msg += libusb_error_name(code);
        }
    }

#   ifdef DEBUG
    LibusbException(const char *file, int line, int code, const char *msg):
        Exception(file, line, msg), errorCode(code)
    {
        if (code) {
            _msg += ": ";
            _msg += libusb_error_name(code);
        }
    }

    LibusbException(const char *file, int line, int code, const std::string &msg):
        Exception(file, line, msg), errorCode(code)
    {
        if (code) {
            _msg += ": ";
            _msg += libusb_error_name(code);
        }
    }
#   endif /* DEBUG */

    virtual
    ~LibusbException() noexcept
    {}
};

#ifdef DEBUG
#define __ADK_USB_THROW_EXCEPTION(__code, __msg, ...) \
    throw adk::LibusbException(__FILE__, __LINE__, __code, __msg, ## __VA_ARGS__)
#else /* DEBUG */
#define __ADK_THROW_EXCEPTION(code, __msg, ...) \
    throw adk::LibusbException(__code, __msg, ## __VA_ARGS__)
#endif /* DEBUG */

/** Throw ADK USB exception.
 * @param code Error code returned by libusb call.
 * @param __msg Message which could be streaming expression. Additional arguments
 *      to the exception class constructor can be specified after the message.
 */
#define ADK_USB_EXCEPTION(__code, __msg, ...) do { \
    std::stringstream __ss; \
    __ss << __msg; \
    __ADK_USB_THROW_EXCEPTION(__code, __ss.str(), ## __VA_ARGS__); \
} while (false)

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
    std::shared_ptr<LibusbDevice>
    OpenDeviceByPid(u16 vendorId, u16 productId)
    {
        libusb_device_handle *hDevice =
            libusb_open_device_with_vid_pid(_ctx, vendorId, productId);
        if (!hDevice) {
            return std::shared_ptr<LibusbDevice>(nullptr);
        }
        return std::shared_ptr<LibusbDevice>(new LibusbDevice(hDevice));
    }
};

} /* namespace adk */

#endif /* ADK_LIBUSB_H_ */
