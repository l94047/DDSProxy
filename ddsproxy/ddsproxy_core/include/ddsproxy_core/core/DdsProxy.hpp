// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#pragma once

#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/thread_pool/pool/SlotThreadPool.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/dynamic/AllowedTopicList.hpp>
#include <ddspipe_core/dynamic/DiscoveryDatabase.hpp>
#include <ddspipe_core/dynamic/ParticipantsDatabase.hpp>
#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>

#include <ddsproxy_core/core/ParticipantFactory.hpp>
#include <ddsproxy_core/configuration/DdsProxyConfiguration.hpp>
#include <ddsproxy_core/library/library_dll.h>

namespace eprosima {
namespace ddsproxy {
namespace core {

/**
 * TODO
 */
class DdsProxy
{
public:

    /**
     * @brief Construct a new DdsProxy object
     *
     * Initialize a whole DdsProxy:
     * - Create its associated AllowedTopicList
     * - Create Participants and add them to \c ParticipantsDatabase
     * - Create the Bridges for (allowed) builtin topics
     *
     * @param [in] configuration : Configuration for the new DDS Proxy
     *
     * @throw \c ConfigurationException in case the yaml inside allowlist is not well-formed
     * @throw \c InitializationException in case \c IParticipants , \c IWriters or \c IReaders creation fails.
     */
    DDSPROXY_CORE_DllAPI DdsProxy(
            const DdsProxyConfiguration& configuration);

    /**
     * @brief Destroy the DdsProxy object
     *
     * Stop the DdsProxy
     * Destroy all Bridges
     * Destroy all Participants
     */
    DDSPROXY_CORE_DllAPI virtual ~DdsProxy() = default;

    // EVENTS
    /**
     * @brief Reload the allowed topic configuration
     *
     * @param [in] configuration : new configuration
     *
     * @return \c RETCODE_OK if configuration has been updated correctly
     * @return \c RETCODE_NO_DATA if new configuration has not changed
     * @return \c RETCODE_ERROR if any other error has occurred
     *
     * @throw \c ConfigurationException in case the new yaml is not well-formed
     */
    DDSPROXY_CORE_DllAPI utils::ReturnCode reload_configuration(
            const DdsProxyConfiguration& configuration);

    /**
     * @brief Start communication in DDS Proxy
     *
     * Enable every topic Bridge.
     *
     * @note this method returns a ReturnCode for future possible errors
     *
     * @return \c RETCODE_OK always
     */
    DDSPROXY_CORE_DllAPI utils::ReturnCode start() noexcept;

    /**
     * @brief Stop communication in DDS Proxy
     *
     * Disable every topic Bridge.
     *
     * @note this method returns a ReturnCode for future possible errors
     *
     * @return \c RETCODE_OK always
     */
    DDSPROXY_CORE_DllAPI utils::ReturnCode stop() noexcept;

protected:

    /**
     * @brief  Create participants and add them to the participants database
     *
     * @throw \c ConfigurationException in case a Participant is not well configured (e.g. No kind)
     * @throw \c InitializationException in case \c IParticipants creation fails.
     */
    void init_participants_();


    DdsProxyConfiguration configuration_;

    std::shared_ptr<ddspipe::core::DiscoveryDatabase> discovery_database_;

    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    std::shared_ptr<ddspipe::core::ParticipantsDatabase> participants_database_;

    std::shared_ptr<utils::SlotThreadPool> thread_pool_;

    std::shared_ptr<ddspipe::core::AllowedTopicList> allowed_topics_;

    std::unique_ptr<ddspipe::core::DdsPipe> ddspipe_;

    ParticipantFactory participant_factory_;
};

} /* namespace core */
} /* namespace ddsproxy */
} /* namespace eprosima */
