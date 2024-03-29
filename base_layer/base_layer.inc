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

#include "base_layer.h"
#include "base_layer_logging.inc"
#include "child_layer.h"

#include "vulkan/vk_layer.h"

#include <cstring>
#include <vector>
#include <assert.h>

#if defined(__ANDROID__)
#include <android/log.h>
#else
#include <stdio.h>
#endif

namespace base_layer
{
const VkLayerProperties kLayerProps = { LAYER_NAME,
                                        VK_HEADER_VERSION_COMPLETE,
                                        VK_MAKE_VERSION(LAYER_VERSION_MAJOR, LAYER_VERSION_MINOR, LAYER_VERSION_PATCH),
                                        LAYER_DESCRIPTION
                                        " Version " GFXRECON_VERSION_STR(LAYER_VERSION_MAJOR) "." GFXRECON_VERSION_STR(
                                            LAYER_VERSION_MINOR) "." GFXRECON_VERSION_STR(LAYER_VERSION_PATCH)
                                            LAYER_VERSION_DESIGNATION };

const std::vector<VkExtensionProperties> kDeviceExtensionProps = { VkExtensionProperties{ "VK_EXT_tooling_info", 1 } };

static const VkLayerInstanceCreateInfo* get_instance_chain_info(const VkInstanceCreateInfo* pCreateInfo,
                                                                VkLayerFunction             func)
{
    const VkLayerInstanceCreateInfo* chain_info =
        reinterpret_cast<const VkLayerInstanceCreateInfo*>(pCreateInfo->pNext);

    while (chain_info &&
           ((chain_info->sType != VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO) || (chain_info->function != func)))
    {
        chain_info = reinterpret_cast<const VkLayerInstanceCreateInfo*>(chain_info->pNext);
    }

    return chain_info;
}

static const VkLayerDeviceCreateInfo* get_device_chain_info(const VkDeviceCreateInfo* pCreateInfo, VkLayerFunction func)
{
    const VkLayerDeviceCreateInfo* chain_info = reinterpret_cast<const VkLayerDeviceCreateInfo*>(pCreateInfo->pNext);

    while (chain_info &&
           ((chain_info->sType != VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO) || (chain_info->function != func)))
    {
        chain_info = reinterpret_cast<const VkLayerDeviceCreateInfo*>(chain_info->pNext);
    }

    return chain_info;
}

static std::mutex                                                    gpdpa_lock;
static std::unordered_map<VkInstance, PFN_GetPhysicalDeviceProcAddr> next_gpdpa;

static void set_instance_next_gpdpa(const VkInstance instance, PFN_GetPhysicalDeviceProcAddr p_next_gpdpa)
{
    assert(instance != VK_NULL_HANDLE);
    std::lock_guard<std::mutex> lock(gpdpa_lock);
    next_gpdpa[instance] = p_next_gpdpa;
}

static PFN_GetPhysicalDeviceProcAddr get_instance_next_gpdpa(const VkInstance instance)
{
    assert(instance != VK_NULL_HANDLE);
    std::lock_guard<std::mutex> lock(gpdpa_lock);
    auto                        it_gpdpa = next_gpdpa.find(instance);
    if (it_gpdpa == next_gpdpa.end())
    {
        return nullptr;
    }
    return it_gpdpa->second;
}

VKAPI_ATTR VkResult VKAPI_CALL base_layer_CreateInstance(const VkInstanceCreateInfo*  pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkInstance*                  pInstance)
{
    VkResult result = VK_ERROR_INITIALIZATION_FAILED;

    VkLayerInstanceCreateInfo* chain_info =
        const_cast<VkLayerInstanceCreateInfo*>(get_instance_chain_info(pCreateInfo, VK_LAYER_LINK_INFO));

    if (chain_info && chain_info->u.pLayerInfo)
    {
        PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;

        if (fpGetInstanceProcAddr)
        {
            PFN_vkCreateInstance fpCreateInstance =
                reinterpret_cast<PFN_vkCreateInstance>(fpGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance"));

            if (fpCreateInstance)
            {
                // Advance the link info for the next element on the chain
                chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

                result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);

                if ((result == VK_SUCCESS) && pInstance && (*pInstance != VK_NULL_HANDLE))
                {
                    InstanceTable* instance_table = add_instance_handle(*pInstance);
                    LoadInstanceTable(fpGetInstanceProcAddr, *pInstance, instance_table);

                    result = layer_CreateInstance(pCreateInfo, pAllocator, pInstance);

                    if (result == VK_SUCCESS)
                    {
                        auto fpNextGetPhysicalDeviceProcAddr = reinterpret_cast<PFN_GetPhysicalDeviceProcAddr>(
                            fpGetInstanceProcAddr(*pInstance, "vk_layerGetPhysicalDeviceProcAddr"));
                        set_instance_next_gpdpa(*pInstance, fpNextGetPhysicalDeviceProcAddr);
                    }
                    else
                    {
                        remove_instance_handle(*pInstance);
                    };
                }
            }
        }
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL base_layer_CreateDevice(VkPhysicalDevice             physicalDevice,
                                                       const VkDeviceCreateInfo*    pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkDevice*                    pDevice)
{
    VkResult                 result = VK_ERROR_INITIALIZATION_FAILED;
    VkLayerDeviceCreateInfo* chain_info =
        const_cast<VkLayerDeviceCreateInfo*>(get_device_chain_info(pCreateInfo, VK_LAYER_LINK_INFO));

    if (chain_info && chain_info->u.pLayerInfo)
    {
        instance_dispatch_table* layer_instance = get_instance_handle(physicalDevice);

        PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
        PFN_vkGetDeviceProcAddr   fpGetDeviceProcAddr   = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;

        if (fpGetInstanceProcAddr && fpGetDeviceProcAddr && layer_instance)
        {
            PFN_vkCreateDevice fpCreateDevice =
                reinterpret_cast<PFN_vkCreateDevice>(fpGetInstanceProcAddr(layer_instance->instance, "vkCreateDevice"));

            if (fpCreateDevice)
            {
                // Advance the link info for the next element on the chain
                chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

                result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

                if ((result == VK_SUCCESS) && pDevice && (*pDevice != VK_NULL_HANDLE))
                {
                    DeviceTable* device_table = add_device_handle(*pDevice);
                    LoadDeviceTable(fpGetDeviceProcAddr, *pDevice, device_table);

                    result = layer_CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

                    if (result != VK_SUCCESS)
                    {
                        remove_device_handle(*pDevice);
                    }
                }
            }
        }
    }

    return result;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetProcAddr(const char* pName)
{
    PFN_vkVoidFunction result = nullptr;

    if (pName)
    {
        if (!strcmp("vkCreateInstance", pName))
        {
            result = reinterpret_cast<PFN_vkVoidFunction>(base_layer_CreateInstance);
        }
        else if (!strcmp("vkCreateDevice", pName))
        {
            result = reinterpret_cast<PFN_vkVoidFunction>(base_layer_CreateDevice);
        }
        else if (!strcmp("vkGetInstanceProcAddr", pName))
        {
            result = reinterpret_cast<PFN_vkVoidFunction>(layer_GetInstanceProcAddr);
        }
        else if (!strcmp("vkGetDeviceProcAddr", pName))
        {
            result = reinterpret_cast<PFN_vkVoidFunction>(layer_GetDeviceProcAddr);
        }
        else if (!strcmp("vkEnumerateInstanceExtensionProperties", pName))
        {
            result = reinterpret_cast<PFN_vkVoidFunction>(EnumerateInstanceExtensionProperties);
        }
        else if (!strcmp("vkEnumerateDeviceExtensionProperties", pName))
        {
            result = reinterpret_cast<PFN_vkVoidFunction>(EnumerateDeviceExtensionProperties);
        }
        else if (!strcmp("vkEnumerateInstanceLayerProperties", pName))
        {
            result = reinterpret_cast<PFN_vkVoidFunction>(EnumerateInstanceLayerProperties);
        }
        else if (!strcmp("vkEnumerateDeviceLayerProperties", pName))
        {
            result = reinterpret_cast<PFN_vkVoidFunction>(EnumerateDeviceLayerProperties);
        }
    }

    return result;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL base_layer_GetInstanceProcAddr(VkInstance instance, const char* pName)
{
    PFN_vkVoidFunction result = GetProcAddr(pName);

    if (!result && instance != VK_NULL_HANDLE)
    {
        instance_dispatch_table* instance_table = get_instance_handle(instance);
        if ((instance_table != nullptr) && (instance_table->dispatch_table.GetInstanceProcAddr != nullptr))
        {
            result = instance_table->dispatch_table.GetInstanceProcAddr(instance, pName);
        }
    }

    return result;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL base_layer_GetDeviceProcAddr(VkDevice device, const char* pName)
{
    PFN_vkVoidFunction result = nullptr;

    if (device != VK_NULL_HANDLE)
    {
        result = GetProcAddr(pName);

        if (!result)
        {
            device_dispatch_table* device_table = get_device_handle(device);
            if ((device_table != nullptr) && (device_table->dispatch_table.GetDeviceProcAddr != nullptr))
            {
                result = device_table->dispatch_table.GetDeviceProcAddr(device, pName);
            }
        }
    }

    return result;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL base_layer_GetPhysicalDeviceProcAddr(VkInstance instance, const char* pName)
{
    PFN_vkVoidFunction result = nullptr;

    if (instance != VK_NULL_HANDLE)
    {
        PFN_GetPhysicalDeviceProcAddr next_gpdpa = get_instance_next_gpdpa(instance);
        if (next_gpdpa != nullptr)
        {
            result = next_gpdpa(instance, pName);
        }
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(VkPhysicalDevice       physicalDevice,
                                                                  const char*            pLayerName,
                                                                  uint32_t*              pPropertyCount,
                                                                  VkExtensionProperties* pProperties)
{
    VkResult result = VK_SUCCESS;

    if ((pLayerName != nullptr) && (std::strcmp(pLayerName, kLayerProps.layerName) == 0))
    {
        if (pPropertyCount != nullptr)
        {
            uint32_t extension_count = static_cast<uint32_t>(kDeviceExtensionProps.size());

            if (pProperties == nullptr)
            {
                *pPropertyCount = extension_count;
            }
            else
            {
                if ((*pPropertyCount) < extension_count)
                {
                    result          = VK_INCOMPLETE;
                    extension_count = *pPropertyCount;
                }
                else if ((*pPropertyCount) > extension_count)
                {
                    *pPropertyCount = extension_count;
                }

                for (uint32_t i = 0; i < extension_count; ++i)
                {
                    pProperties[i] = kDeviceExtensionProps[i];
                }
            }
        }
    }
    else
    {
        // If this function was not called with the layer's name, we expect to dispatch down the chain to obtain the ICD
        // provided extensions.
        // In order to screen out unsupported extensions, we always query the chain
        // twice, and remove those that are present from the count.
        instance_dispatch_table* instance_table            = get_instance_handle(physicalDevice);
        uint32_t                 downstream_property_count = 0;

        result = instance_table->dispatch_table.EnumerateDeviceExtensionProperties(
            physicalDevice, pLayerName, &downstream_property_count, nullptr);
        if (result != VK_SUCCESS)
        {
            return result;
        }

        std::vector<VkExtensionProperties> downstream_properties(downstream_property_count);
        result = instance_table->dispatch_table.EnumerateDeviceExtensionProperties(
            physicalDevice, pLayerName, &downstream_property_count, downstream_properties.data());
        if (result != VK_SUCCESS)
        {
            return result;
        }

        // Output the reduced count or the reduced extension list:
        if (pProperties == nullptr)
        {
            *pPropertyCount = static_cast<uint32_t>(downstream_properties.size());
        }
        else
        {
            if (*pPropertyCount < static_cast<uint32_t>(downstream_properties.size()))
            {
                result = VK_INCOMPLETE;
            }
            *pPropertyCount = std::min(*pPropertyCount, static_cast<uint32_t>(downstream_properties.size()));
            std::copy(downstream_properties.begin(), downstream_properties.begin() + *pPropertyCount, pProperties);
        }
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(const char*            pLayerName,
                                                                    uint32_t*              pPropertyCount,
                                                                    VkExtensionProperties* pProperties)
{
    VkResult result = VK_SUCCESS;

    if (pLayerName && (std::strcmp(pLayerName, kLayerProps.layerName) == 0))
    {
        if (pPropertyCount != nullptr)
        {
            *pPropertyCount = 0;
        }
    }
    else
    {
        result = VK_ERROR_LAYER_NOT_PRESENT;
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(uint32_t*          pPropertyCount,
                                                                VkLayerProperties* pProperties)
{
    VkResult result = VK_SUCCESS;

    if (pProperties == nullptr)
    {
        if (pPropertyCount != nullptr)
        {
            *pPropertyCount = 1;
        }
    }
    else
    {
        if ((pPropertyCount != nullptr) && (*pPropertyCount >= 1))
        {
            std::memcpy(pProperties, &kLayerProps, sizeof(kLayerProps));
            *pPropertyCount = 1;
        }
        else
        {
            result = VK_INCOMPLETE;
        }
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceLayerProperties(VkPhysicalDevice   physicalDevice,
                                                              uint32_t*          pPropertyCount,
                                                              VkLayerProperties* pProperties)
{
    return EnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}

} // namespace base_layer

extern "C"
{

    VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct)
    {
        assert(pVersionStruct != NULL);
        assert(pVersionStruct->sType == LAYER_NEGOTIATE_INTERFACE_STRUCT);

        // Fill in the function pointers if our version is at least capable of having the structure contain them.
        if (pVersionStruct->loaderLayerInterfaceVersion >= 2)
        {
            pVersionStruct->pfnGetInstanceProcAddr       = layer_GetInstanceProcAddr;
            pVersionStruct->pfnGetDeviceProcAddr         = layer_GetDeviceProcAddr;
            pVersionStruct->pfnGetPhysicalDeviceProcAddr = base_layer::base_layer_GetPhysicalDeviceProcAddr;
        }

        if (pVersionStruct->loaderLayerInterfaceVersion > CURRENT_LOADER_LAYER_INTERFACE_VERSION)
        {
            pVersionStruct->loaderLayerInterfaceVersion = CURRENT_LOADER_LAYER_INTERFACE_VERSION;
        }

        return VK_SUCCESS;
    }

    // The following two functions are not directly invoked by the desktop loader, which instead uses the function
    // pointers returned by the negotiate function.
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* pName)
    {
        return layer_GetInstanceProcAddr(instance, pName);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* pName)
    {
        return layer_GetDeviceProcAddr(device, pName);
    }

    VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice       physicalDevice,
                                                                        const char*            pLayerName,
                                                                        uint32_t*              pPropertyCount,
                                                                        VkExtensionProperties* pProperties)
    {
        assert(physicalDevice == VK_NULL_HANDLE);
        return base_layer::EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
    }

    VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char*            pLayerName,
                                                                          uint32_t*              pPropertyCount,
                                                                          VkExtensionProperties* pProperties)
    {
        return base_layer::EnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
    }

    VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t*          pPropertyCount,
                                                                      VkLayerProperties* pProperties)
    {
        return base_layer::EnumerateInstanceLayerProperties(pPropertyCount, pProperties);
    }

    VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice   physicalDevice,
                                                                    uint32_t*          pPropertyCount,
                                                                    VkLayerProperties* pProperties)
    {
        assert(physicalDevice == VK_NULL_HANDLE);
        return base_layer::EnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
    }

} // extern "C"
