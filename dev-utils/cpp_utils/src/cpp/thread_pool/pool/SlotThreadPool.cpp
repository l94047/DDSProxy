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
 * @file SlotThreadPool.cpp
 *
 * This file contains class SlotThreadPool implementation.
 */

#include <cpp_utils/exception/ValueNotAllowedException.hpp>
#include <cpp_utils/utils.hpp>

#include <cpp_utils/thread_pool/pool/SlotThreadPool.hpp>


namespace eprosima {
namespace utils {

SlotThreadPool::SlotThreadPool(
        const uint32_t n_threads)
    : number_of_threads_(n_threads)
    , enabled_(false)
{
    logDebug(UTILS_THREAD_POOL, "Creating Thread Pool with " << n_threads << " threads.");
}

SlotThreadPool::~SlotThreadPool()
{
    disable();
    // Disable queue in case it has not been stopped yet.
    //task_queue_.disable();
    task_queue_priority_0.disable();
    task_queue_priority_1.disable();

    for (auto& thread : threads_)
    {
        thread.join();
    }
}

void SlotThreadPool::enable() noexcept
{
    if (!enabled_.exchange(true))
    {
        // Execute threads
        for (uint32_t i = 0; i < number_of_threads_; ++i)
        {
            threads_.emplace_back(
                CustomThread(
                    std::bind(&SlotThreadPool::thread_routine_, this)));
        }
    }
}

void SlotThreadPool::disable() noexcept
{
    if (enabled_.exchange(false))
    {
        // Disable Task Queue, so threads will stop eventually when their current task is finished
        // task_queue_.disable();
        task_queue_priority_0.disable();
        task_queue_priority_1.disable();

        for (auto& thread : threads_)
        {
            thread.join();
        }

        threads_.clear();
    }
}

void SlotThreadPool::emit(
        const TaskId& task_id)
{
    // Lock to access the slot map
    std::lock_guard<std::mutex> lock(slots_mutex_);

    auto it = slots_.find(task_id);

    if (it == slots_.end())
    {
        throw utils::ValueNotAllowedException(STR_ENTRY << "Slot " << task_id << " not registered.");
    }
    else
    {
        task_queue_priority_0.produce(it->first);
    }
}

void SlotThreadPool::emit_by_priority(
        const TaskId& task_id,
        const unsigned int priority_id)
{
    // Lock to access the slot map
    std::lock_guard<std::mutex> lock(slots_mutex_);

    auto it = slots_.find(task_id);

    if (it == slots_.end())
    {
        throw utils::ValueNotAllowedException(STR_ENTRY << "Slot " << task_id << " not registered.");
    }
    else
    {
        if(priority_id == 0) {
            task_queue_priority_0.produce(it->first);
        }else if (priority_id == 1)
        {
             task_queue_priority_1.produce(it->first);
        }else
        {
            throw utils::ValueNotAllowedException(STR_ENTRY << "Slot " << priority_id << " not allowed.");
        }
        logDebug(UTILS_THREAD_POOL, "Task: " << task_id << " join into queue :" << priority_id);
    }
}

void SlotThreadPool::slot(
        const TaskId& task_id,
        Task&& task)
{
    // Lock to access the slot map
    std::lock_guard<std::mutex> lock(slots_mutex_);

    auto it = slots_.find(task_id);

    if (it != slots_.end())
    {
        throw utils::ValueNotAllowedException(STR_ENTRY << "Slot " << task_id << " already exists.");
    }
    else
    {
        slots_.insert(std::make_pair(task_id, std::move(task)));
    }
}

utils::event::AwakeReason SlotThreadPool::wait_all_consumed(
        const utils::Duration_ms& timeout /* = 0 */)
{
    return task_queue_priority_0.wait_all_consumed(timeout);
}

void SlotThreadPool::thread_routine_()
{
    logDebug(UTILS_THREAD_POOL, "Starting thread routine: " << std::this_thread::get_id() << ".");

    try
    {
        while (true)
        {
            logDebug(UTILS_THREAD_POOL, "Thread: " << std::this_thread::get_id() << " free, getting new callback.");

            TaskId task_id;
            if(!task_queue_priority_0.queue_.BothEmpty())
            {
                task_id = task_queue_priority_0.consume();
            }else
            {
                task_id = task_queue_priority_1.consume(1);
                // logDebug(UTILS_THREAD_POOL, task_id);
                if(task_id == 0){
                    continue;
                }
            }

            // TaskId task_id = task_queue_.consume();

            // Lock to access the slot map
            slots_mutex_.lock();

            auto it = slots_.find(task_id);
            // Check the slot is correct
            if (it == slots_.end())
            {
                utils::tsnh(STR_ENTRY << "Slot in Queue must be stored in slots register");
            }

            Task& task = it->second;

            slots_mutex_.unlock();

            logDebug(UTILS_THREAD_POOL, "Thread: " << std::this_thread::get_id() << " executing callback.");
            task();
        }
    }
    catch (const utils::DisabledException& e)
    {
        logDebug(UTILS_THREAD_POOL, "Stopping thread: " << std::this_thread::get_id() << ".");
    }
}

} /* namespace utils */
} /* namespace eprosima */
