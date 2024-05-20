#!/bin/bash

# CMake Utils
cd ~/DDSProxy
mkdir -p build/cmake_utils
cd build/cmake_utils
cmake ~/DDSProxy/dev-utils/cmake_utils -DCMAKE_INSTALL_PREFIX=~/DDSProxy/install -DCMAKE_PREFIX_PATH=~/Fast-DDS/install
cmake --build . --target install

# C++ Utils
cd ~/DDSProxy
mkdir -p build/cpp_utils
cd build/cpp_utils
cmake ~/DDSProxy/dev-utils/cpp_utils -DCMAKE_INSTALL_PREFIX=~/DDSProxy/install -DCMAKE_PREFIX_PATH=~/Fast-DDS/install
cmake --build . --target install

# ddspipe_core
cd ~/DDSProxy
mkdir build/ddspipe_core
cd build/ddspipe_core
cmake ~/DDSProxy/ddspipe/ddspipe_core -DCMAKE_INSTALL_PREFIX=~/DDSProxy/install -DCMAKE_PREFIX_PATH=~/Fast-DDS/install
cmake --build . --target install

# ddspipe_participants
cd ~/DDSProxy
mkdir build/ddspipe_participants
cd build/ddspipe_participants
cmake ~/DDSProxy/ddspipe/ddspipe_participants -DCMAKE_INSTALL_PREFIX=~/DDSProxy/install -DCMAKE_PREFIX_PATH=~/Fast-DDS/install
cmake --build . --target install

# ddspipe_yaml
cd ~/DDSProxy
mkdir build/ddspipe_yaml
cd build/ddspipe_yaml
cmake ~/DDSProxy/ddspipe/ddspipe_yaml -DCMAKE_INSTALL_PREFIX=~/DDSProxy/install -DCMAKE_PREFIX_PATH=~/Fast-DDS/install
cmake --build . --target install

# ddsproxy_core
cd ~/DDSProxy
mkdir build/ddsproxy_core
cd build/ddsproxy_core
cmake ~/DDSProxy/ddsproxy/ddsproxy_core -DCMAKE_INSTALL_PREFIX=~/DDSProxy/install -DCMAKE_PREFIX_PATH=~/Fast-DDS/install
cmake --build . --target install

# ddsproxy_yaml
cd ~/DDSProxy
mkdir build/ddsproxy_yaml
cd build/ddsproxy_yaml
cmake ~/DDSProxy/ddsproxy/ddsproxy_yaml -DCMAKE_INSTALL_PREFIX=~/DDSProxy/install -DCMAKE_PREFIX_PATH=~/Fast-DDS/install
cmake --build . --target install

# ddsproxy_main
cd ~/DDSProxy
mkdir build/ddsproxy_main
cd build/ddsproxy_main
cmake ~/DDSProxy/ddsproxy/ddsproxy_main -DCMAKE_INSTALL_PREFIX=~/DDSProxy/install -DCMAKE_PREFIX_PATH=~/Fast-DDS/install
cmake --build . --target install
