# Preprocessor macros

## Description

_android-vulkan_ project is using the following preprocessor macros for compilation:

* `ANDROID_NATIVE_MODE_PORTRAIT` or `ANDROID_NATIVE_MODE_LANDSCAPE`
* `ANDROID_VULKAN_DEBUG`
* `ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS`
* `ANDROID_VULKAN_STRICT_MODE`

Macros can be specified in the project's `CMakeLists.txt` file in the section:

```cmake
target_compile_definitions ( android-vulkan
    PRIVATE

    ...
)
```

## `ANDROID_NATIVE_MODE_PORTRAIT` or `ANDROID_NATIVE_MODE_LANDSCAPE`

This macros is responsible for correct mapping between _presentation engine_ and _Vulkan_ output orientation. In most cases application must not introduce additional overhead on memory bandwidth on the mobile devices. So it's recommended to output final images with no additional work for _presentation engine_. On _Android_ you have to adjust two parameters for the project.

**First** you have to choose correct macro

* `ANDROID_NATIVE_MODE_PORTRAIT` - your target device is smartphone with portrait screen orientation
* `ANDROID_NATIVE_MODE_LANDSCAPE` - your target device is tablet-ish device with landscape screen orientation

**Second** you have to choose correct mode in the `AndroidManifest.xml`

```xml
<manifest ...
    ...>

    <application

        ...

        >

        <activity

            ...

            android:screenOrientation=<your native orientation>

            ...

            >

            ...

        </activity>
    </application>
</manifest>
```

where `<your native orientation>` can be

* `"portrait"` - your target device is smartphone with portrait screen orientation
* `"landscape"` - your target device is tablet-ish device with landscape screen orientation

For more information please refer to the following guidelines:

* [Vulkan Mobile Best Practice - Appropriate Use of Surface Rotation](https://community.arm.com/developer/tools-software/graphics/b/blog/posts/appropriate-use-of-surface-rotation)
* [Appropriate use of surface rotation](https://github.com/KhronosGroup/Vulkan-Samples/blob/master/samples/performance/surface_rotation/surface_rotation_tutorial.md)

## `ANDROID_VULKAN_DEBUG`

This macro enables custom mechanism for reporting leaked _Vulkan_ ojects. Also the macro is used for additional debug output in the [_Logcat™_](logcat.md).

## `ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS`

This macro enables [_Khronos_ validation layer](https://developer.android.com/ndk/guides/graphics/validation-layer#apk-containing-layers). This introduces additional overhead in terms of execution. Macro is usefull in the develop time. Also validation layer can be disabled if current layer implementation contains unavoidable bugs and interferes with some features of the framework.

## `ANDROID_VULKAN_STRICT_MODE`

This macro is responsible for invocation of the `accert` instruction in the following situations:

* [_Khronos_ validation layer](https://developer.android.com/ndk/guides/graphics/validation-layer#apk-containing-layers) detects some errors or suboptimal execution
* It's detected some memory leak of the _Vulkan_'s objects
