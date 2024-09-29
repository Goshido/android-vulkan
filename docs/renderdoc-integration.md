# _RenderDoc_ integration

The project supports integration with [_RenderDoc v1.34_](https://renderdoc.org/). But there is a catch. This tool implies additional limitations to hardware features:

- `VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT`
- `VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT`

You could refer to [this thread](https://github.com/baldurk/renderdoc/issues/2681) which covers example of such limitations.

So it's needed to make proper build of the project:

- disable [_Vulkan Validation layers_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) feature: `ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS`. [How to](preprocessor-macros.md#macro-android-vulkan-enable-vulkan-validation-layers)
- activate [_RenderDoc v1.34_](https://renderdoc.org/) compatibility feature: `ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION`. [How to](preprocessor-macros.md#macro-android-vulkan-enable-render-doc-integration)

For example the `CMakeLists.txt` should look like this:

```cmake
...

# See docs/preprocessor-macros.md
target_compile_definitions ( android-vulkan
    PRIVATE
#    -DANDROID_ENABLE_TRACE
    -DANDROID_NATIVE_MODE_PORTRAIT
#    -DANDROID_VULKAN_DEBUG
    -DANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION
#    -DANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS
#    -DANDROID_VULKAN_STRICT_MODE
    -DVK_NO_PROTOTYPES
    -DVK_USE_PLATFORM_ANDROID_KHR
)

...
```

**Note:** The application will crash without _RenderDoc_ after using `ANDROID_VULKAN_ENABLE_RENDER_DOC_INTEGRATION`. It's expected. The reason for this is that application will try to find and call _Vulkan_ debug marker and debug group functions. Most likely they will not be available on device.

That's it.

## Reported issues

Name | Link | Status
--- | --- | ---
spirv-cross.exe error when editing shaders | [#3353](https://github.com/baldurk/renderdoc/issues/3353) | ✔️ Fixed
_Vulkan HLSL_ with source edit issue | [#3425](https://github.com/baldurk/renderdoc/issues/3425) | 🛡️ _DXC_ issue
Can't compile shaders in Edit mode | [#3448](https://github.com/baldurk/renderdoc/issues/3448) | ⚡ Wont fix
