/* /ADK/src/lib/resources.cpp
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012-2013, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file resources.cpp
 * Embedded resources management.
 */

#include <adk.h>

using namespace adk;

namespace {

/** Global map for all attached resources. */
std::map<std::string, ResourceDesc> g_resMap;

} /* anonymous namespace */

internal::ResourceDeclarator::ResourceDeclarator(const char *name,
                                                 const void *start,
                                                 const void *end)
{
    size_t size = static_cast<const u8 *>(end) - static_cast<const u8 *>(start);
    g_resMap.insert(std::pair<std::string, ResourceDesc>(name, ResourceDesc(start, size)));
}

ResourceDesc
adk::GetResource(const char *name)
{
    auto it = g_resMap.find(name);
    if (it == g_resMap.end()) {
        ADK_EXCEPTION(InvalidParamException, "Resource not found: " << name);
    }
    return it->second;
}
