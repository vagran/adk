/* This file is a part of 'ADK' project.
 * Copyright (c) 2012-2015, Artyom Lebedev <artyom.lebedev@gmail.com>
 * All rights reserved.
 * See COPYING file for copyright details.
 */

/** @file scheduler.h
 * TODO insert description here.
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

namespace adk {

/** Maximal number of tasks allowed to schedule. Override before including this
 * file if necessary.
 */
#ifndef SCHEDULER_MAX_TASKS
#define SCHEDULER_MAX_TASKS     10
#endif

class Scheduler {
public:
    /** Scheduled task handler. Returns non-zero delay to reschedule task with or
     * zero to terminate the task.
     */
    typedef u16 (*TaskHandler)();

    typedef u8 TaskId;

    /** Schedule task for deferred execution.
     *
     * @return True if scheduled, false if failed.
     */
    bool
    ScheduleTask(TaskHandler handler, u16 delay);

    /** Cancel previously scheduled task.
     *
     * @return True if unscheduled, false if already terminated.
     */
    bool
    UnscheduleTask(TaskHandler handler);

    /** Schedule polling round. Should be called only from interrupts. */
    inline void
    SchedulePoll()
    {
        pollPending = true;
    }

    /** Run main loop. Never returns. Should be called after the application
     * initialized. Interrupts are enabled in this method.
     */
    void
    Run() __NORETURN;

private:
    /** Task descriptor. */
    struct Task {
        /** Non-zero if scheduled. */
        u16 delay;
        TaskHandler handler;
    };

    /** Task descriptors pool. */
    Task tasks[SCHEDULER_MAX_TASKS];
    /** Number of scheduler ticks passed from previous tasks run. */
    u16 ticks;

    struct {
        /** Any interrupt can set this flag to indicate that scheduler polling
         * round should be executed.
         */
        u8 pollPending:1;
    };


    /** Process scheduled tasks. */
    void
    Poll();

} __PACKED;

/** The application must define this function which is periodically called in
 * the scheduler main loop when interrupt occurs or additional polling rounds
 * are scheduled.
 */
void
PollFunc();

/** The application may define callback which is called atomically with sleep
 * entering by the scheduler. The callback may prevent from sleeping when it is
 * not desired (e.g. during ADC conversion). To use this feature the application
 * should define SCHEDULER_CHECK_SLEEPING_ALLOWED symbol which must the function
 * name to call for sleep permission checking.
 * @return True to enable sleeping, false to prevent from sleeping.
 */
#ifdef SCHEDULER_CHECK_SLEEPING_ALLOWED
bool
SCHEDULER_CHECK_SLEEPING_ALLOWED();
#endif

} /* namespace adk */

#endif /* SCHEDULER_H_ */
