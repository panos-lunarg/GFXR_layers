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

#define LAYER_NAME "VK_LAYER_LUNARG_342"
#define LAYER_VERSION_MAJOR 0
#define LAYER_VERSION_MINOR 1
#define LAYER_VERSION_PATCH 0
#define LAYER_DESCRIPTION "342 layer"
#define LAYER_VERSION_DESIGNATION "-dev"

#include "base_layer/base_layer.inc"
#include <inttypes.h>

VKAPI_ATTR VkResult VKAPI_CALL layer_CreateInstance(const VkInstanceCreateInfo*  pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkInstance*                  pInstance)
{
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL layer_CreateDevice(VkPhysicalDevice             physicalDevice,
                                                  const VkDeviceCreateInfo*    pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkDevice*                    pDevice)
{
    base_layer::base_layer_print_info("%s()", __func__);
    VkBaseOutStructure* current_struct =
        reinterpret_cast<VkBaseOutStructure*>(const_cast<VkDeviceCreateInfo*>(pCreateInfo))->pNext;
    while (current_struct != nullptr)
    {
        base_layer::base_layer_print_info("  current_struct->sType: %u", current_struct->sType);

        if (current_struct->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES)
        {
            base_layer::base_layer_print_info(
                "  current_struct->sType: VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES");
            VkPhysicalDeviceBufferDeviceAddressFeatures* features =
                reinterpret_cast<VkPhysicalDeviceBufferDeviceAddressFeatures*>(current_struct);
            base_layer::base_layer_print_info("  features->bufferDeviceAddress: %u", features->bufferDeviceAddress);
            base_layer::base_layer_print_info("  features->bufferDeviceAddressCaptureReplay: %u",
                                              features->bufferDeviceAddressCaptureReplay);
            base_layer::base_layer_print_info("  features->bufferDeviceAddressMultiDevice: %u",
                                              features->bufferDeviceAddressMultiDevice);

            features->bufferDeviceAddressCaptureReplay = true;
        }
        else if (current_struct->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR)
        {
            base_layer::base_layer_print_info(
                "  current_struct->sType: VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR");
            auto accel_struct_features =
                reinterpret_cast<VkPhysicalDeviceAccelerationStructureFeaturesKHR*>(current_struct);
            base_layer::base_layer_print_info("  accel_struct_features->accelerationStructure: %u",
                                              accel_struct_features->accelerationStructure);
            base_layer::base_layer_print_info("  accel_struct_features->accelerationStructureCaptureReplay: %u",
                                              accel_struct_features->accelerationStructureCaptureReplay);
            base_layer::base_layer_print_info("  accel_struct_features->accelerationStructureIndirectBuild: %u",
                                              accel_struct_features->accelerationStructureIndirectBuild);
            base_layer::base_layer_print_info("  accel_struct_features->accelerationStructureHostCommands: %u",
                                              accel_struct_features->accelerationStructureHostCommands);
            base_layer::base_layer_print_info(
                "  accel_struct_features->descriptorBindingAccelerationStructureUpdateAfterBind: %u",
                accel_struct_features->descriptorBindingAccelerationStructureUpdateAfterBind);

            accel_struct_features->accelerationStructureCaptureReplay = true;
        }
        else if (current_struct->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR)
        {
            base_layer::base_layer_print_info(
                "  current_struct->sType: VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR");
            auto rt_pipeline_features =
                reinterpret_cast<VkPhysicalDeviceRayTracingPipelineFeaturesKHR*>(current_struct);
            base_layer::base_layer_print_info("  rt_pipeline_features->rayTracingPipeline: %u",
                                              rt_pipeline_features->rayTracingPipeline);
            base_layer::base_layer_print_info(
                "  rt_pipeline_features->rayTracingPipelineShaderGroupHandleCaptureReplay: %u",
                rt_pipeline_features->rayTracingPipelineShaderGroupHandleCaptureReplay);
            base_layer::base_layer_print_info(
                "  rt_pipeline_features->rayTracingPipelineShaderGroupHandleCaptureReplayMixed: %u",
                rt_pipeline_features->rayTracingPipelineShaderGroupHandleCaptureReplayMixed);
            base_layer::base_layer_print_info("  rt_pipeline_features->rayTracingPipelineTraceRaysIndirect: %u",
                                              rt_pipeline_features->rayTracingPipelineTraceRaysIndirect);
            base_layer::base_layer_print_info("  rt_pipeline_features->rayTraversalPrimitiveCulling: %u",
                                              rt_pipeline_features->rayTraversalPrimitiveCulling);

            rt_pipeline_features->rayTracingPipelineShaderGroupHandleCaptureReplay = true;
        }
        current_struct = current_struct->pNext;
    }

    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL layer_vkAllocateMemory(VkDevice                     device,
                                                             const VkMemoryAllocateInfo*  pAllocateInfo,
                                                             const VkAllocationCallbacks* pAllocator,
                                                             VkDeviceMemory*              pMemory)
{
    base_layer::device_dispatch_table* device_table = base_layer::get_device_handle(device);
    if (device_table && device_table->dispatch_table.AllocateMemory)
    {
        base_layer::base_layer_print_info("AllocateMemory size: %" PRIu64, pAllocateInfo->allocationSize);

        VkBaseOutStructure* current_struct =
            reinterpret_cast<VkBaseOutStructure*>(const_cast<VkMemoryAllocateInfo*>(pAllocateInfo))->pNext;
        while (current_struct != nullptr)
        {
            if (current_struct->sType == VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO)
            {
                VkMemoryAllocateFlagsInfo* alloc_flags_info =
                    reinterpret_cast<VkMemoryAllocateFlagsInfo*>(current_struct);

                base_layer::base_layer_print_info("  VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO");
                base_layer::base_layer_print_info("  alloc_flags_info->flags: 0x%x", alloc_flags_info->flags);

                if ((alloc_flags_info->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT) ==
                    VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT)
                {
                    alloc_flags_info->flags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;
                }
                break;
            }

            current_struct = current_struct->pNext;
        }

        return device_table->dispatch_table.AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
    }
    else
    {
        base_layer::base_layer_print_error("AllocateMemory was not initialized in the dispatch table");
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL layer_GetProcAddr(const char* pName)
{
    PFN_vkVoidFunction result = nullptr;

    if (pName)
    {
        if (!strcmp(pName, "vkAllocateMemory"))
        {
            result = (PFN_vkVoidFunction)layer_vkAllocateMemory;
        }
    }

    return result;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL layer_GetInstanceProcAddr(VkInstance instance, const char* pName)
{
    (void)instance;

    PFN_vkVoidFunction result = layer_GetProcAddr(pName);

    if (!result)
    {
        result = base_layer::base_layer_GetInstanceProcAddr(instance, pName);
    }

    return result;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL layer_GetDeviceProcAddr(VkDevice device, const char* pName)
{
    (void)device;

    PFN_vkVoidFunction result = layer_GetProcAddr(pName);

    if (!result)
    {
        result = base_layer::base_layer_GetDeviceProcAddr(device, pName);
    }

    return result;
}
