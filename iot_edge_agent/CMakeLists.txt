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
    ./src/modbus.c
    ./src/dlt_698.c
    ./src/protocol_manager.c
    ./src/channel_model.c
    ./src/iotea_client.c
    ./src/iot_main.c
    ./src/serial_io.c
    ./src/utils.c
)

set(iot_edge_agent_h_files
    ../parson/parson.h
    ./inc/modbus.h
    ./inc/dlt_698.h
    ./inc/protocol_manager.h
    ./inc/channel_model.h
    ./inc/iotea_client.h
    ./inc/iot_main.h
    ./inc/serial_io.h
    ./inc/utils.h
)

include_directories(${AZURE_C_SHARED_UTILITY_INCLUDES})
include_directories(${SHARED_UTIL_INC_FOLDER})
include_directories(${MQTT_INC_FOLDER})
include_directories(${IOTHUB_CLIENT_INC_FOLDER})
include_directories(${LIGHTMODBUS_INC_FOLDER})
include_directories(${DLT698_INC_FOLDER})

set(IOT_MODEBUS_INC_FOLDER ${CMAKE_CURRENT_LIST_DIR}/inc CACHE INTERNAL "this is what needs to be included if using iothub_client lib" FORCE)
include_directories(${IOT_MODEBUS_INC_FOLDER})

add_executable(
    iot-edge-agent
    ${iot_edge_agent_c_files}
    ${iot_edge_agent_h_files}
)

target_link_libraries(
    iot-edge-agent
    lightmodbus
    dlt698
    iothub_client
    umqtt
)

target_link_libraries(
    iot-edge-agent
    ${OPENSSL_LIBRARIES}
)
