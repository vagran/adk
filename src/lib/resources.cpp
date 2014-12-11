/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
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
std::map<std::string, ResourceDesc> *g_resMap = nullptr;

/** Helper class for releasing resources map. */
class MapReleaser {
public:
    ~MapReleaser()
    {
        if (g_resMap) {
            delete g_resMap;
            g_resMap = nullptr;
        }
    }
};

MapReleaser g_mapReleaser;

} /* anonymous namespace */

adk::internal::ResourceDeclarator::ResourceDeclarator(const char *name,
                                                      const void *start,
                                                      const void *end)
{
    if (!g_resMap) {
        g_resMap = new std::map<std::string, ResourceDesc>();
    }
    size_t size = static_cast<const u8 *>(end) - static_cast<const u8 *>(start);
    g_resMap->insert(std::pair<std::string, ResourceDesc>(name, ResourceDesc(start, size)));
}

ResourceDesc
adk::GetResource(const char *name)
{
    if (!g_resMap) {
        g_resMap = new std::map<std::string, ResourceDesc>();
    }
    auto it = g_resMap->find(name);
    if (it == g_resMap->end()) {
        ADK_EXCEPTION(InvalidParamException, "Resource not found: " << name);
    }
    return it->second;
}
