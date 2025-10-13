# _RenderDoc_ integration

## <a id="table-of-content">Table of content</a>

- [_Brief_](#brief)
- [_Reported issues_](#issues)

## <a id="brief">Brief</a>

The project supports integration with [_RenderDoc v1.40_](https://renderdoc.org/). But there is a catch. This tool implies additional limitations to hardware features:

- `VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT`
- `VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT`

You could refer to [this thread](https://github.com/baldurk/renderdoc/issues/2681) which covers example of such limitations.

So it's needed to make proper build of the project:

- disable [_Vulkan Validation layers_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) feature: `AV_ENABLE_VVL`. [How to](preprocessor-macros.md#macro-av-enable-vvl)
- activate [_RenderDoc v1.40_](https://renderdoc.org/) compatibility feature: `AV_ENABLE_RENDERDOC`. [How to](preprocessor-macros.md#macro-av-enable-renderdoc)

For example the `CMakeLists.txt` should look like this:

```cmake
...

# See docs/preprocessor-macros.md
target_compile_definitions ( android-vulkan
    PRIVATE
    AV_ARM_NEON
#    AV_DEBUG
    AV_ENABLE_RENDERDOC
#    AV_ENABLE_TRACE
#    AV_ENABLE_VVL
    AV_FREETYPE
    AV_NATIVE_MODE_PORTRAIT
#    AV_STRICT_MODE
    LUA_32BITS
    VK_NO_PROTOTYPES
    VK_USE_PLATFORM_ANDROID_KHR
)

...
```

**Note:** The application will crash without _RenderDoc_ after using `AV_ENABLE_RENDERDOC`. It's expected. The reason for this is that application will try to find and call _Vulkan_ debug marker and debug group functions. Most likely they will not be available on device.

That's it.

[↬ table of content ⇧](#table-of-content)

## <a id="issues">Reported issues</a>

Name | Link | Status
--- | --- | ---
spirv-cross.exe error when editing shaders | [#3353](https://github.com/baldurk/renderdoc/issues/3353) | ✔️ Fixed
_Vulkan HLSL_ with source edit issue | [#3425](https://github.com/baldurk/renderdoc/issues/3425) | 🛡️ _DXC_ issue
Can't compile shaders in Edit mode | [#3448](https://github.com/baldurk/renderdoc/issues/3448) | 🛡️ _DXC_ issue
_RenderDoc 1.39_ closing connection during _Vulkan_ capture inspection | [#3643](https://github.com/baldurk/renderdoc/issues/3643) | ✔️ Fixed

[↬ table of content ⇧](#table-of-content)
