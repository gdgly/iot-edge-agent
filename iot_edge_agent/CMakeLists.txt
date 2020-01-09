# Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8.11)

if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()

compileAsC99()

include_directories(../parson)

set(iot_edge_agent_c_files
    ../parson/parson.c
    ./src/serial_port.c
    ./src/modbus_rtu.c
    ./src/protocol_manager.c
    ./src/iot_model.c
    ./src/iotea_client.c
    ./src/main.c
)

set(iot_edge_agent_h_files
    ../parson/parson.h
    ./inc/serial_port.h
    ./inc/modbus_rtu.h
    ./inc/protocol_manager.h
    ./inc/iot_model.h
    ./inc/iotea_client.h
    ./inc/main.h
)

include_directories(${AZURE_C_SHARED_UTILITY_INCLUDES})
include_directories(${SHARED_UTIL_INC_FOLDER})
include_directories(${MQTT_INC_FOLDER})
include_directories(${IOTHUB_CLIENT_INC_FOLDER})
include_directories(${LIGHTMODBUS_INC_FOLDER})
set(IOT_MODEBUS_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using iothub_client lib" FORCE)
include_directories(${IOT_MODEBUS_INC_FOLDER})

add_executable(
    iot-edge-agent
    ${iot_edge_agent_c_files}
    ${iot_edge_agent_h_files}
)

target_link_libraries(
    iot-edge-agent
    # aziotsharedutil
    # pthread
    lightmodbus
    iothub_client
    umqtt
)

target_link_libraries(
    iot-edge-agent
    ${OPENSSL_LIBRARIES}
)
