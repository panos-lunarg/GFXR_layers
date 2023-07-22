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

#ifndef CHILD_LAYER_H_
#define CHILD_LAYER_H_

#include "vulkan/vulkan.h"
#include "vulkan/vk_layer.h"

// The following functions must be defined by each implemented layer
VKAPI_ATTR VkResult VKAPI_CALL layer_CreateInstance(const VkInstanceCreateInfo*  pCreateInfo,
                                                    const VkAllocationCallbacks* pAllocator,
                                                    VkInstance*                  pInstance);

VKAPI_ATTR VkResult VKAPI_CALL layer_CreateDevice(VkPhysicalDevice             physicalDevice,
                                                  const VkDeviceCreateInfo*    pCreateInfo,
                                                  const VkAllocationCallbacks* pAllocator,
                                                  VkDevice*                    pDevice);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL layer_GetInstanceProcAddr(VkInstance instance, const char* pName);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL layer_GetDeviceProcAddr(VkDevice device, const char* pName);

// Rest functions are up to the layer to implement or not
VKAPI_ATTR VkResult VKAPI_CALL layer_QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);

VKAPI_ATTR VkResult VKAPI_CALL layer_QueueSubmit(VkQueue             queue,
                                                 uint32_t            submitCount,
                                                 const VkSubmitInfo* pSubmits,
                                                 VkFence             fence);

#endif // CHILD_LAYER_H_