/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See LICENSE file for copyright details.
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
    ToString(std::stringstream &ss) const;

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

    LibusbDevice(libusb_device_handle *hDevice);

    ~LibusbDevice();

    /** Get address assigned to the device. */
    u8
    GetAddress();

    /** Reset the device. */
    void
    Reset();

    /** Write data to device.
     *
     * @param data Data buffer.
     * @param size Size of data in bytes.
     * @param timeout Timeout in milliseconds. Zero fo unlimited timeout.
     * @return Number of bytes actually transferred.
     */
    size_t
    Write(const void *data, size_t size, int timeout = 0);

    /** Read data from device.
     * @param data Data buffer.
     * @param size Desired transfer size in bytes.
     * @param timeout Timeout in milliseconds. Zero fo unlimited timeout.
     * @return Number of bytes actually transferred.
     */
    size_t
    Read(void *data, size_t size, int timeout = 0);

private:
    friend class LibusbCtx;

    libusb_device_handle *_hDevice;
    libusb_device *_device;
};

/** Wrapper class for libusb context. Should be used for all USB operations. */
class LibusbCtx {
public:
    LibusbCtx();

    ~LibusbCtx();

    /** Open device by vendor and product ID.
     * @return Pointer to the device. @a nullptr if opening failed.
     */
    LibusbDevice::Handle
    OpenDeviceByPid(u16 vendorId, u16 productId);

private:
    libusb_context *_ctx = nullptr;
};

} /* namespace adk */

#endif /* ADK_LIBUSB_H_ */
