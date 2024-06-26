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

#include <ddspipe_yaml/yaml_configuration_tags.hpp>
#include <ddspipe_yaml/Yaml.hpp>
#include <ddspipe_yaml/YamlManager.hpp>
#include <ddspipe_yaml/YamlReader.hpp>

#include <ddsproxy_core/configuration/DdsProxyConfiguration.hpp>

#include <ddsproxy_yaml/YamlReaderConfiguration.hpp>

namespace eprosima {
namespace ddsproxy {
namespace yaml {

core::DdsProxyConfiguration
YamlReaderConfiguration::load_ddsproxy_configuration(
        const Yaml& yml)
{
    try
    {
        ddspipe::yaml::YamlReaderVersion version;
        // Get version if present
        if (ddspipe::yaml::YamlReader::is_tag_present(yml, ddspipe::yaml::VERSION_TAG))
        {
            version = ddspipe::yaml::YamlReader::get<ddspipe::yaml::YamlReaderVersion>(yml, ddspipe::yaml::VERSION_TAG,
                            ddspipe::yaml::YamlReaderVersion::LATEST);

            switch (version)
            {
                case ddspipe::yaml::YamlReaderVersion::V_4_0:
                case ddspipe::yaml::YamlReaderVersion::LATEST:
                    break;

                case ddspipe::yaml::YamlReaderVersion::V_1_0:
                case ddspipe::yaml::YamlReaderVersion::V_2_0:
                case ddspipe::yaml::YamlReaderVersion::V_3_0:
                case ddspipe::yaml::YamlReaderVersion::V_3_1:
                default:

                    throw eprosima::utils::ConfigurationException(
                              utils::Formatter() <<
                                  "The yaml configuration version " << version <<
                                  " is no longer supported. Please update to v4.0.");
                    break;
            }
        }
        else
        {
            // Get default version
            version = default_yaml_version();
            logWarning(DDSPROXY_YAML,
                    "No version of yaml configuration given. Using version " << version << " by default. " <<
                    "Add " << ddspipe::yaml::VERSION_TAG << " tag to your configuration in order to not break compatibility " <<
                    "in future releases.");
        }

        logInfo(DDSPROXY_YAML, "Loading DDSProxy configuration with version: " << version << ".");

        // Load DDS Proxy Configuration
        core::DdsProxyConfiguration proxy_configuration =
                ddspipe::yaml::YamlReader::get<core::DdsProxyConfiguration>(yml, version);

        return proxy_configuration;
    }
    catch (const std::exception& e)
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDS Proxy configuration from yaml:\n " << e.what());
    }
}

core::DdsProxyConfiguration
YamlReaderConfiguration::load_ddsproxy_configuration_from_file(
        const std::string& file_path)
{
    Yaml yml;

    // Load file
    try
    {
        yml = ddspipe::yaml::YamlManager::load_file(file_path);
    }
    catch (const std::exception& e)
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDSProxy configuration from file: <" << file_path <<
                      "> :\n " << e.what());
    }

    if (yml.IsNull())
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDSProxy configuration from file: <" << file_path <<
                      "> :\n " << "yaml node is null.");
    }

    return YamlReaderConfiguration::load_ddsproxy_configuration(yml);
}

ddspipe::yaml::YamlReaderVersion YamlReaderConfiguration::default_yaml_version()
{
    return ddspipe::yaml::V_4_0;
}

} // namespace yaml
} // namespace ddsproxy
} // namespace eprosima
