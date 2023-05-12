LOCAL_PATH := $(call my-dir)
SRC_DIR := ..
GFXR_DIR := $(SRC_DIR)/../..

include $(CLEAR_VARS)
LOCAL_MODULE := perfetto_lib
LOCAL_SRC_FILES += $(SRC_DIR)/../external/perfetto/sdk/perfetto.cc
LOCAL_C_INCLUDES += $(SRC_DIR)/external/perfetto/perfetto/sdk
LOCAL_CPPFLAGS += -std=c++17 -Wall -Werror -Wno-unused-function -Wno-unused-const-variable
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libperfetto_layer
LOCAL_SRC_FILES += $(SRC_DIR)/../perfetto/perfetto_layer.cpp
LOCAL_SRC_FILES += $(SRC_DIR)/../perfetto/perfetto_tracing_categories.cpp
# This is needed for the generated dispatch table
LOCAL_SRC_FILES += $(GFXR_DIR)/../framework/util/logging.cpp
LOCAL_C_INCLUDES += $(GFXR_DIR)/framework
LOCAL_C_INCLUDES += $(GFXR_DIR)/external/Vulkan-Headers/include
LOCAL_C_INCLUDES += $(SRC_DIR)/external/perfetto/sdk
LOCAL_C_INCLUDES += $(SRC_DIR)/external/perfetto/sdk
LOCAL_C_INCLUDES += $(SRC_DIR)/perfetto
LOCAL_STATIC_LIBRARIES += perfetto_lib
LOCAL_CPPFLAGS += -std=c++17 -DVK_PROTOTYPES -DVK_ENABLE_BETA_EXTENSIONS -Wall -Werror -Wno-unused-function -Wno-unused-const-variable -fexceptions
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
