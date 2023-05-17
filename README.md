# Vulkan layer template

The purpose of this repository is to facilitate the process of writing new Vulkan layers. It attempts to achieve this by providing the minimum boilerplate Vulkan API code that is needed for that purpose.
It does that by providing the source files in the `base_layer` and `generated` folders. It does not attempt to generate any compilation rules and this is left entirely to the user.

## How to use

Each new layer should be placed in a new folder inside the `layers` subfolder.

Each implemented layer must `#include` the `base_layer/base_layer.inc` file in its main `.cpp` source file. Before doing so must `#define` a few macros that concern the layer's name are used when `vkEnumerateInstanceExtensionProperties` and `vkEnumerateDeviceExtensionProperties` are invoked by the Vulkan loader. These macros are the following along with some example definitions:
```
#define LAYER_NAME "VK_LAYER_LUNARG_gfxreconstruct_perfetto_layer"
#define LAYER_VERSION_MAJOR 0
#define LAYER_VERSION_MINOR 1
#define LAYER_VERSION_PATCH 0
#define LAYER_DESCRIPTION "GFXReconstruct perfetto layer"
#define LAYER_VERSION_DESIGNATION "-dev"
```

Each implemented layer must define the functions that are declared in the `base_layer/child_layer.h` header file which are the following:
```
VkResult layer_CreateInstance(const VkInstanceCreateInfo*  pCreateInfo,
                              const VkAllocationCallbacks* pAllocator,
                              VkInstance*                  pInstance);

VkResult layer_CreateDevice(VkPhysicalDevice             physicalDevice,
                            const VkDeviceCreateInfo*    pCreateInfo,
                            const VkAllocationCallbacks* pAllocator,
                            VkDevice*                    pDevice);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL layer_GetInstanceProcAddr(VkInstance instance, const char* pName);
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL layer_GetDeviceProcAddr(VkDevice device, const char* pName);

extern "C"
{
    VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct);
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* pName);
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* pName);
}
```

**Dispatch table**

A dispatch table is generated and populated and can be used by implemented layers in order to forward the Vulkan API commands they implement downchain to the next layer or to the Vulkan driver.
For example forwarding the `vkQueueSubmit` command:
```
base_layer::device_dispatch_table* device_table = base_layer::get_device_handle(queue);
if (device_table && device_table->dispatch_table.QueueSubmit)
{
    result = device_table->dispatch_table.QueueSubmit(queue, submitCount, pSubmits, fence);
}

return result;
```

The included implemented example layer in `layers/perfetto` is an example on how to use the boilerplate code and provides compilation rules for Linux and Android.

### File structure

- `base_layer/base_layer.inc`
This file contains the implementation of the boilerplace functions a vulkan layer should implement. This file should be `#include`'d in each implemented layer's main `.cpp` file.

- `base_layer/base_layer.h`
Contains the function declarations defined in `base_layer.inc`. Also contains the definitions for the instance and device dispatch tables.

- `base_layer/child_layer.h`
This file contains the function declarations each layer must implement.

- `base_layer/logging.h`
Contains function declarations for functions that can be used for printing.

- `base_layer/platform_types.h`
Contains auxiliary declarations and definitions required by the `generated_vulkan_dispatch_table.h` file

- `generated/generated_vulkan_dispatch_table.h`
Contains definitions of the vulkan instance and device table structures. This file is generated from the `vk.xml` registry file.

- `layers/perfetto`
An implemented layer that can be used as an example.

## Regenerating the dispatch tables

Python scripts are provided that generate the dispatch tables for instance and device Vulkan functions. The generation is based on the `vk.xml` registry provided in the Vulkan-Headers repository and is included as a git submodule.
To regenerate the dispatch table:
```
cd generated
python3 generate_vulkan.py
```

## Building

It is not attempted to provide general purpose compilation rules or to generate rules for each layer.
Each layer must define their own compilation rules for each platform they wish to target.

### Cloning

```
git clone git@github.com:panos-lunarg/GFXR_layers.git
cd GFXR_layers
git submodule update --recursive --init
```

## Requirements
`C++17` is required as `shared_mutex` is used for synchronizing access to the dispatch tables.
