# Preprocessor macros

## Description

_android-vulkan_ project is using the following preprocessor macros for compilation:

* [`AV_ENABLE_TRACE`](#macro-av-enable-trace)
* [`AV_NATIVE_MODE_PORTRAIT` or `AV_NATIVE_MODE_LANDSCAPE`](#macro-av-native-mode)
* [`AV_DEBUG`](#macro-av-debug)
* [`AV_ENABLE_RENDERDOC`](#macro-av-enable-renderdoc)
* [`AV_ENABLE_VVL`](#macro-av-enable-vvl)
* [`AV_STRICT_MODE`](#macro-av-strict-mode)

Macros can be specified in the project's `CMakeLists.txt` file in the section:

```cmake
target_compile_definitions ( android-vulkan
    PRIVATE

    ...
)
```

## <a id="macro-av-enable-trace">`AV_ENABLE_TRACE`</a>

This macro is responsible for _CPU_ tracing activation. _Android Studio_ has internal tool for trace collection and analysis. It is described [here](https://developer.android.com/studio/profile/record-traces).

## <a id="macro-av-native-mode">`AV_NATIVE_MODE_PORTRAIT` or `AV_NATIVE_MODE_LANDSCAPE`</a>

This macros are responsible for correct mapping between _presentation engine_ and _Vulkan_ output orientation. Those macros are mutually exclusive. In most cases application must not introduce additional overhead on memory bandwidth on the mobile devices. So it's recommended to output final images with no additional work for _presentation engine_. On _Android_ you have to adjust two parameters for the project.

**First** you have to choose correct macro

* `AV_NATIVE_MODE_PORTRAIT` - your target device is smartphone with portrait screen orientation
* `AV_NATIVE_MODE_LANDSCAPE` - your target device is tablet-ish device with landscape screen orientation

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

* [_Vulkan_ Mobile Best Practice - Appropriate Use of Surface Rotation](https://community.arm.com/developer/tools-software/graphics/b/blog/posts/appropriate-use-of-surface-rotation)
* [Appropriate use of surface rotation](https://github.com/KhronosGroup/Vulkan-Samples/tree/main/samples/performance/surface_rotation)

## <a id="macro-av-debug">`AV_DEBUG`</a>

This macro enables custom mechanism for reporting leaked:

- sound emitter objects

This macro also reports about:

- potential inaccurate results during physics simulation

Also the macro is used for additional debug output in the [_Logcat™_](logcat.md).

## <a id="macro-av-enable-renderdoc">`AV_ENABLE_RENDERDOC`</a>

This macro enables integration with [_RenderDoc_](https://renderdoc.org/). Note that macro is not compatible with [`AV_ENABLE_VVL`](#macro-av-enable-vvl) macro.

**Note:** it's expected that application could work only from [_RenderDoc_](https://renderdoc.org/). Running application outside [_RenderDoc_](https://renderdoc.org/) could produce crash. For more information look [_RenderDoc_ integration](./renderdoc-integration.md).

## <a id="macro-av-enable-vvl">`AV_ENABLE_VVL`</a>

This macro enables [_Khronos_ validation layer](https://developer.android.com/ndk/guides/graphics/validation-layer#apk-containing-layers). This introduces additional overhead in terms of execution. Macro is useful in the develop time. Also validation layer can be disabled if current layer implementation contains unavoidable bugs and interferes with some features of the framework.

Note that macro is not compatible with [`AV_ENABLE_RENDERDOC`](#macro-av-enable-vvl) macro.

## <a id="macro-av-strict-mode">`AV_STRICT_MODE`</a>

This macro is responsible for invocation of the `assert` instruction in the following situations:

* [_VVL_](https://developer.android.com/ndk/guides/graphics/validation-layer#apk-containing-layers) detects some errors or suboptimal execution
* It's detected some memory leak of the _Vulkan_'s objects
* It's detected some memory leak of the sound emitter objects

It's useful because debugger will make "auto breakpoint" during the execution and could allow the developer to inspect callstack, variables, memory and so on.
