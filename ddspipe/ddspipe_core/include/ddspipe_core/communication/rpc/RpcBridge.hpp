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

#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <set>
#include <shared_mutex>

#include <ddspipe_core/communication/Bridge.hpp>
#include <ddspipe_core/communication/rpc/ServiceRegistry.hpp>
#include <ddspipe_core/interface/IWriter.hpp>
#include <ddspipe_core/interface/IReader.hpp>
#include <ddspipe_core/types/topic/rpc/RpcTopic.hpp>

namespace eprosima {
namespace ddspipe {
namespace core {

/**
 * Bridge object manages the communication of a \c RpcTopic.
 *
 * Contains a proxy server and client in each participant.
 *
 * Keeps track of the (actual) servers available at each participant, so the whole bridge can be disabled if no server
 * is available for processing a request (services use RELIABLE & VOLATILE qos by default, so if a request is sent
 * and no server is there to receive it, it will remain unanswered even if a new server appears later).
 *
 */
class RpcBridge : public Bridge
{
public:

    /**
     * RpcBridge constructor by required values
     *
     * In RpcBridge construction, no endpoints are created.
     *
     * @param topic: Topic (service) of which this RpcBridge manages communication
     * @param participant_database: Collection of Participants to manage communication
     * @param payload_pool: Payload Pool that handles the reservation/release of payloads throughout the DDS Proxy
     * @param thread_pool: Shared pool of threads in charge of data transmission.
     *
     * @note Always created disabled, manual enable required. First enable creates all endpoints.
     */
    DDSPIPE_CORE_DllAPI
    RpcBridge(
            const types::RpcTopic& topic,
            const std::shared_ptr<ParticipantsDatabase>& participants_database,
            const std::shared_ptr<PayloadPool>& payload_pool,
            const std::shared_ptr<utils::SlotThreadPool>& thread_pool);

    /**
     * @brief Destructor
     *
     * Before deleting, it calls \c disable.
     * It deletes all the endpoints created in this bridge.
     */
    DDSPIPE_CORE_DllAPI
    virtual ~RpcBridge();

    /**
     * Enable bridge in case it is not enabled and there are (actual) servers available
     * Does nothing if it is already enabled
     *
     * Thread safe
     */
    DDSPIPE_CORE_DllAPI
    void enable() noexcept override;

    /**
     * Disable bridge in case it is not enabled
     * Does nothing if it is disabled
     *
     * Thread safe
     */
    DDSPIPE_CORE_DllAPI
    void disable() noexcept override;

    //! New server discovered -> add to database and enable registry in discoverer participant (in case it was disabled)
    DDSPIPE_CORE_DllAPI
    void discovered_service(
            const types::ParticipantId& server_participant_id,
            const types::GuidPrefix& server_guid_prefix) noexcept;

    //! Server removed -> delete from database (if present) and disable bridge if it was the last server available
    DDSPIPE_CORE_DllAPI
    void removed_service(
            const types::ParticipantId& server_participant_id,
            const types::GuidPrefix& server_guid_prefix) noexcept;

protected:

    /**
     * Creates all proxy clients and servers associated to this bridge.
     *
     * Called only once in execution (controlled by \c init_ flag).
     *
     * @throw InitializationException in case \c IWriters or \c IReaders creation fails.
     */
    void init_nts_(); // throws exception, caught in enable

    /**
     * Create a RTPS Reader in request topic and a RTPS Writer in reply topic.
     *
     * @param participant_id: Participant where proxy server is to be created
     *
     * @throw InitializationException in case \c IWriters or \c IReaders creation fails.
     */
    void create_proxy_server_nts_(
            types::ParticipantId participant_id);

    /**
     * Create a RTPS Reader in reply topic and a RTPS Writer in request topic.
     * Create a \c ServiceRegistry associated to this proxy client.
     *
     * @param participant_id: Participant where proxy client is to be created
     *
     * @throw InitializationException in case \c IWriters or \c IReaders creation fails.
     */
    void create_proxy_client_nts_(
            types::ParticipantId participant_id);

    //! Create slot in the thread pool for this reader
    void create_slot_(
            std::shared_ptr<IReader> reader) noexcept;

    //! Callback to execute when a new cache change is added to this reader
    void data_available_(
            const types::Guid& reader_guid) noexcept;

    /**
     * REQUEST: Take data from request \c reader and send this data through all proxy clients which are in contact
     * with actual servers (service registry enabled).
     *
     * REPLY: Take data from reply \c reader and send it through the proxy server which originally received the request
     * (information present in service registry).
     *
     * Finish execution when no more data is available, or bridge has been disabled (due to servers unavailability or
     * topic being blocked).
     */
    void transmit_(
            std::shared_ptr<IReader> reader) noexcept;

    //! Whether there are any servers in the database
    bool servers_available_() const noexcept;

    //! Flag set to true when proxy clients and servers are created, so it can only be done once
    bool init_;

    //! Proxy servers endpoints
    std::map<types::ParticipantId, std::shared_ptr<IReader>> request_readers_;
    std::map<types::ParticipantId, std::shared_ptr<IWriter>> reply_writers_;

    //! Proxy clients endpoints
    std::map<types::ParticipantId, std::shared_ptr<IReader>> reply_readers_;
    std::map<types::ParticipantId, std::shared_ptr<IWriter>> request_writers_;

    //! Map readers' GUIDs to their associated thread pool tasks, and also keep a task emission flag.
    std::map<types::Guid, std::pair<bool, utils::TaskId>> tasks_map_;

    /**
     * Registry of requests received, with all the information needed to send the future reply back to the requester.
     *
     * There is one per participant, handling the communication of each of them with the servers they are directly
     * in contact with.
     */
    std::map<types::ParticipantId, std::shared_ptr<ServiceRegistry>> service_registries_;

    //! Database keeping track of the (actual) servers available at each participant.
    std::map<types::ParticipantId, std::set<types::GuidPrefix>> current_servers_;

    //! Mutex to prevent simultaneous calls to enable and/or disable
    std::mutex mutex_;

    /**
     * Mutex to guard while the RpcBridge is sending a message so it could not be disabled.
     */
    std::shared_timed_mutex on_transmission_mutex_;

    types::RpcTopic rpc_topic_;

    // Allow operator << to use private variables
    friend std::ostream& operator <<(
            std::ostream&,
            const RpcBridge&);
};

/**
 * @brief \c RpcBridge to stream serialization
 *
 * This method is merely a to_string of a RpcBridge definition.
 * It serialize the RPCtopic
 */
std::ostream& operator <<(
        std::ostream& os,
        const RpcBridge& bridge);

} /* namespace core */
} /* namespace ddspipe */
} /* namespace eprosima */
