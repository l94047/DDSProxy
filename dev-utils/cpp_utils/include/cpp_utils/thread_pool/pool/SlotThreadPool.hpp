// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file SlotThreadPool.hpp
 *
 * This file contains class SlotThreadPool definition.
 */

#pragma once

#include <map>
#include <thread>
#include <vector>

#include <cpp_utils/library/library_dll.h>
#include <cpp_utils/thread_pool/task/Task.hpp>
#include <cpp_utils/thread_pool/task/TaskId.hpp>
#include <cpp_utils/thread_pool/thread/CustomThread.hpp>
#include <cpp_utils/wait/DBQueueWaitHandler.hpp>

namespace eprosima {
namespace utils {

/**
 * This class represents a thread pool that can register tasks inside.
 *
 * This is another implementation of \c ThreadPool but with the difference that if does not contain actual
 * task objects, but only task ids. These ids are much more efficient than actual task objects in order to copy
 * or store them. Each id identifies one and only one task. By adding an id to the queue, the thread that consumes
 * it will execute the task associated, that must be previously registered.
 *
 * @note Qt notation is used for this implementation, so \c emit means to add a task to the queue and
 * \c slot means to register a task.
 *
 * @note This class does not inherit from \c ThreadPool as methods and internal variables are not shared,
 * even when both solve the same problem in similar ways.
 */
class SlotThreadPool
{
public:

    /**
     * @brief Construct a new Slot Thread Pool object
     *
     * This creates the internal threads in the pool and make them wait for tasks.
     * Each thread is executed with function \c thread_routine_ .
     *
     * @param n_threads number of threads in the pool
     */
    CPP_UTILS_DllAPI SlotThreadPool(
            const uint32_t n_threads);

    /**
     * @brief Destroy the Thread Pool object
     *
     * It disables the queue, what makes the threads to stop to finish their tasks and exit.
     */
    CPP_UTILS_DllAPI ~SlotThreadPool();

    /**
     * Enable Slot Thread Pool in case it is not enabled
     * Does nothing if it is already enabled
     */
    CPP_UTILS_DllAPI void enable() noexcept;

    /**
     * Disable Slot Thread Pool in case it is enabled
     * Does nothing if it is already disabled
     *
     * It stops all the threads running, not allowing them to take new tasks.
     * It blocks until every thread has finished executing.
     * It does not remove tasks from queue.
     *
     * @todo this is a first approach, a new design should be taken into account to not block until threads finish
     * when disabling the thread pool, but joining them afterwards.
     */
    CPP_UTILS_DllAPI void disable() noexcept;

    /**
     * @brief Add a task Id (that represents a registered Task) to be executed by the threads in the pool
     *
     * This add \c task_id to the queue, and the task identified will be executed by the threads in the pool.
     *
     * @pre \c task_id must identify a registered task.
     *
     * @param task_id task Id to be added to the queue so task identified is executed.
     */
    CPP_UTILS_DllAPI void emit(
            const TaskId& task_id);

    CPP_UTILS_DllAPI void emit_by_priority(
            const TaskId& task_id,
            const unsigned int priority_id);        

    /**
     * @brief Register a new task identified by a task Id.
     *
     * This method registers a new task that will be executed when its task Id is added to the queue.
     *
     * @param task_id task Id that identifies the task.
     * @param task task to be registered.
     */
    CPP_UTILS_DllAPI void slot(
            const TaskId& task_id,
            Task&& task);

    /**
     * @brief Wait until all queued tasks are executed.
     *
     * This method will wait until all scheduled tasks are executed.
     * In case there is no programmed task at the moment of calling this method, it returns immediately.
     *
     * @param timeout maximum time to wait in milliseconds. If 0, not time limit. [default 0].
     * @return AwakeReason Whether the method returned due to timeout or because all tasks were executed.
     */
    CPP_UTILS_DllAPI utils::event::AwakeReason wait_all_consumed(
            const utils::Duration_ms& timeout = 0);

protected:

    /**
     * @brief This is the function that every thread in the pool executes.
     *
     * This function enters an infinite loop where it \c consume an element <TaskId> from the queue (this means it will
     * wait for an element to be added to the queue in case it is empty, and it will take one if any available).
     * Once a task id is available, it will get the task refering this id and execute it
     * Afterwards it will return to consume another task id.
     * This will be repeated until the queue is disabled, what is communicated by a \c DisabledException .
     */
    void thread_routine_();

    unsigned int number_of_threads_;

    /**
     * @brief Double Queue Wait Handler to store task ids
     *
     * This double queue implement methods \c produce , to add tasks to the queue, and \c consume to wait until any
     * task is available, and return the next task available.
     *
     * It will retrieve tasks in FIFO order.
     * Produce and consume methods are not reciprocally blocking.
     */
//     utils::event::DBQueueWaitHandler<TaskId> task_queue_;

    utils::event::DBQueueWaitHandler<TaskId> task_queue_priority_0;

    utils::event::DBQueueWaitHandler<TaskId> task_queue_priority_1;   

    /**
     * @brief Threads container
     *
     * @note \c CustomThread are used instead of \c std::thread so some extra logic could be added to threads
     * in future implementation (e.g. performance info).
     */
    std::vector<CustomThread> threads_;

    /**
     * @brief Map of tasks indexed by their task Id.
     *
     * This object is protected by the \c slots_mutex_ mutex.
     */
    std::map<TaskId, Task> slots_;

    //! Protects access to \c slots_ .
    std::mutex slots_mutex_;

    //! Whether the object is currently enabled
    std::atomic<bool> enabled_;

};

} /* namespace utils */
} /* namespace eprosima */


