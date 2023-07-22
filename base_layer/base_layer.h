/*
** Copyright (c) 2023 Valve Corporation
** Copyright (c) 2023 LunarG, Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and associated documentation files (the "Software"),
** to deal in the Software without restriction, including without limitation
** the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the
** Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
*/

#ifndef BASE_LAYER_H
#define BASE_LAYER_H

#include "generated/generated_vulkan_dispatch_table.h"

#include "vulkan/vulkan.h"

#include <mutex>
#include <shared_mutex>
#include <unordered_map>

#ifndef LAYER_NAME
#error "LAYER_NAME must be defined"
#endif

#ifndef LAYER_VERSION_MAJOR
#error "LAYER_VERSION_MAJOR must be defined"
#endif

#ifndef LAYER_VERSION_MINOR
#error "LAYER_VERSION_MINOR must be defined"
#endif

#ifndef LAYER_VERSION_PATCH
#error "LAYER_VERSION_PATCH must be defined"
#endif

#ifndef LAYER_DESCRIPTION
#error "LAYER_DESCRIPTION must be defined"
#endif

#ifndef LAYER_VERSION_DESIGNATION
#error "LAYER_VERSION_DESIGNATION must be defined"
#endif

#define GFXRECON_VERSION_STR_EXPAND(x) #x
#define GFXRECON_VERSION_STR(x) GFXRECON_VERSION_STR_EXPAND(x)

namespace base_layer
{
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL base_layer_GetInstanceProcAddr(VkInstance instance, const char* pName);
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL base_layer_GetDeviceProcAddr(VkDevice device, const char* pName);
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL base_layer_GetPhysicalDeviceProcAddr(VkInstance  ourInstanceWrapper,
                                                                              const char* pName);

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(VkPhysicalDevice       physicalDevice,
                                                                  const char*            pLayerName,
                                                                  uint32_t*              pPropertyCount,
                                                                  VkExtensionProperties* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(const char*            pLayerName,
                                                                    uint32_t*              pPropertyCount,
                                                                    VkExtensionProperties* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(uint32_t*          pPropertyCount,
                                                                VkLayerProperties* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceLayerProperties(VkPhysicalDevice   physicalDevice,
                                                              uint32_t*          pPropertyCount,
                                                              VkLayerProperties* pProperties);

VKAPI_ATTR VkResult VKAPI_CALL base_layer_CreateInstance(const VkInstanceCreateInfo*  pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkInstance*                  pInstance);

VKAPI_ATTR VkResult VKAPI_CALL base_layer_CreateDevice(VkPhysicalDevice             physicalDevice,
                                                       const VkDeviceCreateInfo*    pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkDevice*                    pDevice);

extern "C"
{
    VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct);
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* pName);
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* pName);
    VKAPI_ATTR VkResult VKAPI_CALL           vkEnumerateDeviceExtensionProperties(VkPhysicalDevice       physicalDevice,
                                                                                  const char*            pLayerName,
                                                                                  uint32_t*              pPropertyCount,
                                                                                  VkExtensionProperties* pProperties);
    VKAPI_ATTR VkResult VKAPI_CALL           vkEnumerateInstanceExtensionProperties(const char*            pLayerName,
                                                                                    uint32_t*              pPropertyCount,
                                                                                    VkExtensionProperties* pProperties);
    VKAPI_ATTR VkResult VKAPI_CALL           vkEnumerateInstanceLayerProperties(uint32_t*          pPropertyCount,
                                                                                VkLayerProperties* pProperties);
    VKAPI_ATTR VkResult VKAPI_CALL           vkEnumerateDeviceLayerProperties(VkPhysicalDevice   physicalDevice,
                                                                              uint32_t*          pPropertyCount,
                                                                              VkLayerProperties* pProperties);
}

struct instance_dispatch_table
{
    VkInstance    instance;
    InstanceTable dispatch_table;
};

struct device_dispatch_table
{
    VkDevice    device;
    DeviceTable dispatch_table;
};

static std::shared_mutex                                        global_lock;
static std::unordered_map<const void*, instance_dispatch_table> instance_handles;
static std::unordered_map<const void*, device_dispatch_table>   device_handles;

static InstanceTable* add_instance_handle(VkInstance instance)
{
    // Store the instance for use with vkCreateDevice.
    std::unique_lock<std::shared_mutex> lock(global_lock);
    instance_handles[GetDispatchKey(instance)] = { instance, InstanceTable() };
    return &instance_handles[GetDispatchKey(instance)].dispatch_table;
}

static void remove_instance_handle(const void* handle)
{
    std::unique_lock<std::shared_mutex> lock(global_lock);
    auto                                entry = instance_handles.find(GetDispatchKey(handle));
    if (entry != instance_handles.end())
    {
        instance_handles.erase(entry);
    }
}

static instance_dispatch_table* get_instance_handle(const void* handle)
{
    std::shared_lock<std::shared_mutex> lock(global_lock);
    auto                                entry = instance_handles.find(GetDispatchKey(handle));
    return (entry != instance_handles.end()) ? &entry->second : nullptr;
}

static DeviceTable* add_device_handle(VkDevice device)
{
    // Store the instance for use with vkCreateDevice.
    std::unique_lock<std::shared_mutex> lock(global_lock);
    device_handles[GetDispatchKey(device)] = { device, DeviceTable() };
    return &device_handles[GetDispatchKey(device)].dispatch_table;
}

static void remove_device_handle(const void* handle)
{
    std::unique_lock<std::shared_mutex> lock(global_lock);
    auto                                entry = device_handles.find(GetDispatchKey(handle));
    if (entry != device_handles.end())
    {
        device_handles.erase(entry);
    }
}

static device_dispatch_table* get_device_handle(const void* handle)
{
    std::shared_lock<std::shared_mutex> lock(global_lock);
    auto                                entry = device_handles.find(GetDispatchKey(handle));
    return (entry != device_handles.end()) ? &entry->second : nullptr;
}

} // namespace base_layer

#endif // BASE_LAYER_H
