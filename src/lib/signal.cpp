/* /ADK/src/lib/signal.cpp
 *
 * This file is a part of 'ADK' project.
 * Copyright (c) 2012-2014, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file signal.cpp
 * Signals and slots.
 */

#include <adk.h>

using namespace adk;

void
SlotTarget::_RegisterSlot(adk_internal::SlotBase *slot)
{
    std::unique_lock<std::mutex> lock(_slotsMutex);
    _slots.push_back(slot);
}

void
SlotTarget::_ReplaceSlot(adk_internal::SlotBase *slot,
                         adk_internal::SlotBase *newSlot)
{
    std::unique_lock<std::mutex> lock(_slotsMutex);
    for (adk_internal::SlotBase *&slotEntry: _slots) {
        if (slotEntry == slot) {
            slotEntry = newSlot;
            return;
        }
    }
    ASSERT(false);
}

void
SlotTarget::_RemoveSlot(adk_internal::SlotBase *slot)
{
    std::unique_lock<std::mutex> lock(_slotsMutex);
    _slots.remove(slot);
}

SlotTarget::~SlotTarget()
{
    std::unique_lock<std::mutex> lock(_slotsMutex);
    for (adk_internal::SlotBase *slot: _slots) {
        slot->_Unbind();
    }
}
