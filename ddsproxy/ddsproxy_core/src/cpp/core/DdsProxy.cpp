// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DdsProxy.cpp
 *
 */

#include <cpp_utils/Log.hpp>
#include <cpp_utils/exception/ConfigurationException.hpp>
#include <cpp_utils/exception/InitializationException.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/dynamic/AllowedTopicList.hpp>
#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>
#include <ddspipe_core/types/dds/TopicQoS.hpp>

#include <ddsproxy_core/configuration/DdsProxyConfiguration.hpp>
#include <ddsproxy_core/core/DdsProxy.hpp>

namespace eprosima {
namespace ddsproxy {
namespace core {

DdsProxy::DdsProxy(
        const DdsProxyConfiguration& configuration)
    : configuration_(configuration)
    , discovery_database_(new ddspipe::core::DiscoveryDatabase())
    , payload_pool_(new ddspipe::core::FastPayloadPool())
    , participants_database_(new ddspipe::core::ParticipantsDatabase())
    , thread_pool_(std::make_shared<utils::SlotThreadPool>(configuration_.advanced_options.number_of_threads))
{
    logDebug(DDSPROXY, "Creating DDS Proxy.");

    // Check that the configuration is correct
    utils::Formatter error_msg;
    if (!configuration_.is_valid(error_msg))
    {
        throw utils::ConfigurationException(
                  utils::Formatter() <<
                      "Configuration for DDS Proxy is invalid: " << error_msg);
    }

    // Load Participants
    init_participants_();

    // Initialize the DdsPipe
    ddspipe_ = std::unique_ptr<ddspipe::core::DdsPipe>(new ddspipe::core::DdsPipe(
                        configuration_.ddspipe_configuration,
                        discovery_database_,
                        payload_pool_,
                        participants_database_,
                        thread_pool_));

    logDebug(DDSPROXY, "DDS Proxy created.");
}

void DdsProxy::init_participants_()
{
    for (std::pair<types::ParticipantKind,
            std::shared_ptr<ddspipe::participants::ParticipantConfiguration>> participant_config :
            configuration_.participants_configurations)
    {
        std::shared_ptr<ddspipe::core::IParticipant> new_participant =
                participant_factory_.create_participant(
            participant_config.first,
            participant_config.second,
            payload_pool_,
            discovery_database_);

        // create_participant should throw an exception in fail, never return nullptr
        if (!new_participant)
        {
            // Failed to create participant
            throw utils::InitializationException(utils::Formatter()
                          << "Failed to create creating Participant " << participant_config.second->id);
        }

        logInfo(DDSPROXY, "Participant created with id: " << new_participant->id()
                                                           << " and kind " << participant_config.first << ".");

        // Add this participant to the database. If it is repeated it will cause an exception
        try
        {
            participants_database_->add_participant(
                new_participant->id(),
                new_participant);
        }
        catch (const utils::InconsistencyException& )
        {
            throw utils::ConfigurationException(utils::Formatter()
                          << "Participant ids must be unique. The id " << new_participant->id() << " is duplicated.");
        }
    }
}

utils::ReturnCode DdsProxy::reload_configuration(
        const DdsProxyConfiguration& new_configuration)
{
    // Check that the configuration is correct
    utils::Formatter error_msg;
    if (!new_configuration.is_valid(error_msg))
    {
        throw utils::ConfigurationException(
                  utils::Formatter() <<
                      "Configuration for Reload DDS Proxy is invalid: " << error_msg);
    }
    // new_configuration.ddspipe_configuration.master_flag = new_configuration.advanced_options.master_flag;

    // Reload the DdsPipe configuration, since it is the only reconfigurable attribute.
    return ddspipe_->reload_configuration(new_configuration.ddspipe_configuration);
}

utils::ReturnCode DdsProxy::start() noexcept
{
    utils::ReturnCode ret = ddspipe_->enable();
    if (ret == utils::ReturnCode::RETCODE_OK)
    {
        logInfo(DDSPROXY, "Starting DDS Proxy.");
    }

    return ret;
}

utils::ReturnCode DdsProxy::stop() noexcept
{
    utils::ReturnCode ret = ddspipe_->disable();
    if (ret == utils::ReturnCode::RETCODE_OK)
    {
        logInfo(DDSPROXY, "Stopping DDS Proxy.");
    }

    return ret;
}

} /* namespace core */
} /* namespace ddsproxy */
} /* namespace eprosima */
