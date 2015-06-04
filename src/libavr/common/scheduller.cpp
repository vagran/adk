/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file scheduller.cpp
 * TODO insert description here.
 */

#include <adk.h>

using namespace adk;


bool
Scheduler::ScheduleTask(TaskHandler handler, u16 delay)
{
    AtomicSection as;
    for (TaskId id = 0; id < SIZEOF_ARRAY(tasks); id++) {
        if (tasks[id].delay == 0) {
            tasks[id].delay = delay;
            tasks[id].handler = handler;
            return true;
        }
    }
    return false;
}

bool
Scheduler::UnscheduleTask(TaskHandler handler)
{
    AtomicSection as;
    for (TaskId id = 0; id < SIZEOF_ARRAY(tasks); id++) {
        if (tasks[id].handler == handler) {
            tasks[id].delay = 0;
            return TRUE;
        }
    }
    return FALSE;
}

void
Scheduler::Poll()
{
    cli();
    u16 _ticks = ticks;
    ticks = 0;
    sei();

    if (_ticks == 0) {
        return;
    }

    bool pendingSkipped = false;
    for (TaskId id = 0; id < SIZEOF_ARRAY(tasks); id++) {
        Task *task = &tasks[id];

        if (task->delay != 0) {
            if (task->delay <= _ticks) {
                /* Preserve delay during the call to reserve this entry. */
                task->delay = task->handler();
                if (task->delay != 0) {
                    pendingSkipped = true;
                }
            } else {
                task->delay -= _ticks;
                pendingSkipped = true;
            }
        }
    }

    cli();
    if (pendingSkipped && ticks != 0) {
        /* Schedule additional round instantly. */
        pollPending = true;
    }
    sei();
}

void
Scheduler::Run()
{
    while (true) {
        pollPending = false;
        sei();

        Poll();
        PollFunc();

        cli();
        if (!pollPending
#ifdef SCHEDULER_CHECK_SLEEPING_ALLOWED
            && SCHEDULER_CHECK_SLEEPING_ALLOWED()
#endif
            ) {
            AVR_BIT_SET8(MCUCR, SE);
            /* Atomic sleeping. */
            __asm__ volatile ("sei; sleep");
            AVR_BIT_CLR8(MCUCR, SE);
            cli();
        }
    }
}
