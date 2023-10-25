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

LOCAL_PATH := $(call my-dir)
SRC_DIR := ..

include $(CLEAR_VARS)
LOCAL_MODULE := perfetto_lib
LOCAL_SRC_FILES += $(SRC_DIR)/../external/perfetto/sdk/perfetto.cc
LOCAL_C_INCLUDES += $(SRC_DIR)/../external/perfetto/perfetto/sdk
LOCAL_CPPFLAGS += -std=c++17 -Wall -Werror -Wno-unused-function -Wno-unused-const-variable
include $(BUILD_STATIC_LIBRARY)

# include $(CLEAR_VARS)
# LOCAL_MODULE := libVkLayer_gfxreconstruct_perfetto
# LOCAL_SRC_FILES += $(SRC_DIR)/../layers/perfetto/perfetto_layer.cpp
# LOCAL_SRC_FILES += $(SRC_DIR)/../layers/perfetto/perfetto_tracing_categories.cpp
# LOCAL_C_INCLUDES += $(SRC_DIR)/
# LOCAL_C_INCLUDES += $(SRC_DIR)/base_layer
# LOCAL_C_INCLUDES += $(SRC_DIR)/external/Vulkan-Headers/include
# LOCAL_C_INCLUDES += $(SRC_DIR)/external/perfetto/sdk
# LOCAL_C_INCLUDES += $(SRC_DIR)/layers/perfetto
# LOCAL_STATIC_LIBRARIES += perfetto_lib
# LOCAL_CPPFLAGS += -std=c++17 -DVK_PROTOTYPES -DVK_ENABLE_BETA_EXTENSIONS -Wall -Werror -Wno-unused-function -Wno-unused-const-variable -fexceptions
# LOCAL_LDLIBS := -llog
# include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libVkLayer_342
LOCAL_SRC_FILES += $(SRC_DIR)/../layers/342/342_layer.cpp
LOCAL_C_INCLUDES += $(SRC_DIR)/
LOCAL_C_INCLUDES += $(SRC_DIR)/base_layer
LOCAL_C_INCLUDES += $(SRC_DIR)/external/Vulkan-Headers/include
LOCAL_CPPFLAGS += -std=c++17 -DVK_PROTOTYPES -DVK_ENABLE_BETA_EXTENSIONS -Wall -Werror -Wno-unused-function -Wno-unused-const-variable -fexceptions
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
