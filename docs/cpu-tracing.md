# _CPU_ tracing

The project supports _Android Studio_'s [tracing tool](https://developer.android.com/topic/performance/tracing). It's activated via preprocessor macro - [`AV_ENABLE_TRACE`](preprocessor-macros.md#macro-av-enable-trace). The tool requires to make debuggable application to work. So recommended `CMakeLists.txt` configuration is the following:

```cmake
...

# Treat compile warnings as errors
target_compile_options ( android-vulkan PRIVATE
    -fno-exceptions
    -fno-rtti
    -Wall
    -Werror
    -Wextra
    -Wpedantic
#    -Wshadow
)

# See docs/preprocessor-macros.md
target_compile_definitions ( android-vulkan
    PRIVATE
    AV_ENABLE_TRACE
    AV_NATIVE_MODE_PORTRAIT
#    AV_DEBUG
#    AV_ENABLE_RENDERDOC
#    AV_ENABLE_VVL
#    AV_STRICT_MODE
    AV_ARM_NEON
    VK_NO_PROTOTYPES
    VK_USE_PLATFORM_ANDROID_KHR
)

...
```

Also you should select _System Trace_ option:

<img src="./images/tracing-settings.png"/>

---

Example from **2022/08/18**:

<img src="./images/tracing-preview.png"/>
