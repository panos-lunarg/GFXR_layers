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

#define LAYER_NAME "VK_LAYER_LUNARG_gfxreconstruct_perfetto"
#define LAYER_VERSION_MAJOR 0
#define LAYER_VERSION_MINOR 1
#define LAYER_VERSION_PATCH 0
#define LAYER_DESCRIPTION "GFXReconstruct perfetto layer"
#define LAYER_VERSION_DESIGNATION "-dev"

#include "base_layer/base_layer.inc"

#include "perfetto_tracing_categories.h"

#include <sstream>

static void InitializePerfetto()
{
    static bool initialized = false;

    if (!initialized)
    {
        perfetto::TracingInitArgs args;
        args.backends |= perfetto::kInProcessBackend;
        args.backends |= perfetto::kSystemBackend;
        perfetto::Tracing::Initialize(args);
        perfetto::TrackEvent::Register();

        initialized = true;
    }
}

typedef uint64_t(VKAPI_PTR* PFN_vkGetBlockIndexGFXR)();
static PFN_vkGetBlockIndexGFXR GetBlockIndexGFXR_fp = nullptr;

VKAPI_ATTR VkResult VKAPI_CALL layer_CreateInstance(const VkInstanceCreateInfo*  pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkInstance*                  pInstance)
{
    InitializePerfetto();

    base_layer::instance_dispatch_table* instance_table = base_layer::get_instance_handle(*pInstance);

    if (instance_table && instance_table->dispatch_table.GetInstanceProcAddr)
    {
        GetBlockIndexGFXR_fp = reinterpret_cast<PFN_vkGetBlockIndexGFXR>(
            instance_table->dispatch_table.GetInstanceProcAddr(*pInstance, "GetBlockIndexGFXR"));

        if (GetBlockIndexGFXR_fp)
        {
            base_layer::base_layer_print_info("Pointer to GetBlockIndexGFXR was acquired\n");
        }
        else
        {
            base_layer::base_layer_print_error("Pointer to GetBlockIndexGFXR was NOT acquired\n");
        }
    }
    else
    {
        base_layer::base_layer_print_error("Retrieving instance table for instance %p failed\n", *pInstance);
    }

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL layer_CreateDevice(VkPhysicalDevice             physicalDevice,
                                                  const VkDeviceCreateInfo*    pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkDevice*                    pDevice)
{
    base_layer::device_dispatch_table* device_table = base_layer::get_device_handle(*pDevice);
    if (device_table)
    {
        if (device_table->dispatch_table.QueueSubmit == nullptr)
        {
            base_layer::base_layer_print_error(
                "Pointer to QueueSubmit in dispatch table for device %p has not been initialized\n", *pDevice);
        }

        if (device_table->dispatch_table.QueuePresentKHR == nullptr)
        {
            base_layer::base_layer_print_error(
                "Pointer to QueuePresentKHR in dispatch table for device %p has not been initialized\n", *pDevice);
        }
    }
    else
    {
        base_layer::base_layer_print_error("Retrieving device table for device %p failed\n", *pDevice);
    }

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL layer_QueueSubmit(VkQueue             queue,
                                                 uint32_t            submitCount,
                                                 const VkSubmitInfo* pSubmits,
                                                 VkFence             fence)
{
    // Forward function to next layer / driver
    VkResult                           result       = VK_SUCCESS;
    base_layer::device_dispatch_table* device_table = base_layer::get_device_handle(queue);
    if (device_table && device_table->dispatch_table.QueueSubmit)
    {
        result = device_table->dispatch_table.QueueSubmit(queue, submitCount, pSubmits, fence);
    }

    if (pSubmits && pSubmits->commandBufferCount && pSubmits->pCommandBuffers)
    {
        const uint64_t block_index = GetBlockIndexGFXR_fp ? GetBlockIndexGFXR_fp() : 0;
        TRACE_EVENT_INSTANT("GFXR", "vkQueueSubmit", [&](perfetto::EventContext ctx) {
            ctx.AddDebugAnnotation(perfetto::DynamicString{ "vkQueueSubmit:" }, block_index);

            for (uint32_t i = 0; i < pSubmits->commandBufferCount; ++i)
            {
                std::stringstream cmd_buf_ptr;
                cmd_buf_ptr << std::hex << pSubmits->pCommandBuffers[i];
                ctx.AddDebugAnnotation<perfetto::DynamicString, perfetto::DynamicString>(
                    perfetto::DynamicString{ "vkCommandBuffer: " + std::to_string(i) },
                    perfetto::DynamicString(cmd_buf_ptr.str()));
            }
        });
    }
    else
    {
        TRACE_EVENT_INSTANT("GFXR", "vkQueueSubmit (empty)", [&](perfetto::EventContext ctx) {});
    }

    return result;
}

VKAPI_ATTR VkResult VKAPI_CALL layer_QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
{
    // Forward function to next layer / driver
    VkResult                           result       = VK_SUCCESS;
    base_layer::device_dispatch_table* device_table = base_layer::get_device_handle(queue);
    if (device_table && device_table->dispatch_table.QueuePresentKHR)
    {
        result = device_table->dispatch_table.QueuePresentKHR(queue, pPresentInfo);
    }

    const uint64_t block_index = GetBlockIndexGFXR_fp ? GetBlockIndexGFXR_fp() : 0;
    TRACE_EVENT_INSTANT("GFXR", "QueuePresent", [&](perfetto::EventContext ctx) {
        ctx.AddDebugAnnotation(perfetto::DynamicString{ "QueuePresent:" }, block_index);
    });

    return result;
}

VKAPI_PTR void VKAPI_CALL layer_FrameBoundaryANDROID(VkDevice device, VkSemaphore semaphore, VkImage image)
{
    // Forward function to next layer / driver
    base_layer::device_dispatch_table* device_table = base_layer::get_device_handle(device);
    if (device_table && device_table->dispatch_table.FrameBoundaryANDROID)
    {
        device_table->dispatch_table.FrameBoundaryANDROID(device, semaphore, image);
    }

    const uint64_t block_index = GetBlockIndexGFXR_fp ? GetBlockIndexGFXR_fp() : 0;
    TRACE_EVENT_INSTANT("GFXR", "FrameBoundaryANDROID", [&](perfetto::EventContext ctx) {
        ctx.AddDebugAnnotation(perfetto::DynamicString{ "FrameBoundaryANDROID:" }, block_index);
    });
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL layer_GetProcAddr(const char* pName)
{
    PFN_vkVoidFunction result = nullptr;

    if (pName)
    {
        if (!strcmp(pName, "vkQueueSubmit"))
        {
            result = (PFN_vkVoidFunction)layer_QueueSubmit;
        }
        else if (!strcmp(pName, "vkQueuePresentKHR"))
        {
            result = (PFN_vkVoidFunction)layer_QueuePresentKHR;
        }
        else if (!strcmp(pName, "vkFrameBoundaryANDROID"))
        {
            result = (PFN_vkVoidFunction)layer_FrameBoundaryANDROID;
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
