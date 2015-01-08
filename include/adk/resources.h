/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file resources.h
 * Embedded resources.
 */


#ifndef RESOURCES_H_
#define RESOURCES_H_

namespace adk {

/** Resource descriptor. */
class ResourceDesc {
public:
    ResourceDesc(const void *data, size_t size):
        _data(data), _size(size)
    {}

    /** Get pointer to the resource raw data. */
    const void *
    GetData() const
    {
        return _data;
    }

    /** Get the resource raw data size. */
    size_t
    GetSize() const
    {
        return _size;
    }

#   ifndef ADK_PLATFORM_AVR
    std::string
    GetString() const
    {
        return std::string(static_cast<const char *>(_data), _size);
    }
#   endif /* ADK_PLATFORM_AVR */

private:
    const void *_data;
    size_t _size;
};

/** Get resource by its file name.
 * @throws InvalidParamException if no resource with the specified name exists.
 */
ResourceDesc
GetResource(const char *name);

#ifndef ADK_PLATFORM_AVR

namespace internal {

/** Helper class for resources declaration. */
class ResourceDeclarator {
public:
    ResourceDeclarator(const char *name, const void *start, const void *end);
};

} /* namespace internal */

/** Internal macro for declaring attached resources.
 * @param __id Internal symbolic name.
 * @param __name File name of the resource.
 * @param __start Start address of the resource data.
 * @param __end End address of the resource data.
 */
#define ADK_DECL_RESOURCE(__id, __name, __start, __end) \
    namespace { internal::ResourceDeclarator __CONCAT(__res_decl_, __id) \
        (__name, __start, __end); }

#else /* ADK_PLATFORM_AVR */
//XXX not implemented
ADK_DECL_RESOURCE(__id, __name, __start, __end)
#endif

#include <auto_adk_res.h>

} /* namespace adk */

#endif /* RESOURCES_H_ */
