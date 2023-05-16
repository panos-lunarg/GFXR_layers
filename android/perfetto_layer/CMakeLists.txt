###############################################################################
# Copyright (c) 2023 Valve Corporation
# Copyright (c) 2023 LunarG, Inc.
# All rights reserved
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# Author: LunarG Team
# Author: AMD Developer Tools Team
# Description: CMake script for framework util target
###############################################################################

cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

include(../cmake-config/PlatformConfig.cmake)

add_library(perfetto STATIC ${CMAKE_SOURCE_DIR}/../../external/perfetto/sdk/perfetto.cc)
target_link_libraries(perfetto log)

add_library(VkLayer_gfxreconstruct_perfetto SHARED "")

target_sources(VkLayer_gfxreconstruct_perfetto
               PRIVATE
                    ${CMAKE_SOURCE_DIR}/../../layers/perfetto/perfetto_layer.cpp
                    ${CMAKE_SOURCE_DIR}/../../layers/perfetto/perfetto_tracing_categories.cpp
)

target_compile_definitions(VkLayer_gfxreconstruct_perfetto PRIVATE VK_NO_PROTOTYPES VK_ENABLE_BETA_EXTENSIONS)

target_include_directories(VkLayer_gfxreconstruct_perfetto
                           PUBLIC
                               ${CMAKE_SOURCE_DIR}/../../
                               ${CMAKE_SOURCE_DIR}/../../base_layer
                               ${CMAKE_SOURCE_DIR}/../../external/perfetto/sdk
                               ${CMAKE_SOURCE_DIR}/../../external/Vulkan-Headers/include
                               )

target_link_libraries(VkLayer_gfxreconstruct_perfetto perfetto)
