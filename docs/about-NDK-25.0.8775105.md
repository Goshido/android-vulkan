# About _NDK 25.0.8775105_ and _Android Studio_ debugger

At the moment **[2022/07/20]** any breakpoint inside native _C++_ code doesn't work if current _Android NDK_ has version `25.0.8775105`. Build environment:

* _Windows Vista_+ or _Monjaro KDE_
* _Android Studio Chipmunk | 2021.2.1 Patch 1_
* _Android Studio Gradle Plugin 7.2.1_
* _Android SDK 11.0 (API level 30)_
* _Android SDK Build-Tools 33.0.0_
* _Android SDK Platform-Tools 33.0.2_
* _CMake 3.18.1_
* _DirectX Shader Compiler 1.7.2207.10002_
* _Kotlin 1.7.10_
* _Kotlin Gradle plugin 1.7.10_
* _Vulkan Validation Layers_ `58f83806b0be820246cffa9a31550b7ab856e5d4`
* _Lua_ `8426d9b4d4df1da3c5b2d759e509ae1c50a86667`
* _Gradle 7.5-bin_
* Real _Android 11_ device with _Vulkan 1.1.131_ support
* [_ARM Neon A64_](https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/neon-programmers-guide-for-armv8-a/introducing-neon-for-armv8-a) native support

## History

The [video](https://drive.google.com/file/d/1MWUNINCH7XUyCcTAD9Mih4dmx7lZAbhd/view?usp=sharing) with issue live demonstration.

[The issue](https://issuetracker.google.com/issues/239589378) which was addressed to the _Google_ guys.

---

**[2022-07-22]** _CMake 3.22.1_ is officially supported by _Android Studio_. After changing project to it and fixing new error with precompiled libraries the breakpoints begin to work.

```gradle
android {
    ...

    ndkVersion "25.0.8775105"

    ...

    // Solving '2 files found with path' error [CMake 3.22.1]. Based on ideas from
    // https://dtuto.com/questions/8657/2-files-found-with-path-lib-arm64-v8a-libc-shared-so-from-inputs-react-native
    packagingOptions {
        jniLibs.pickFirsts.add("lib/arm64-v8a/libLua.so")
        jniLibs.pickFirsts.add("lib/arm64-v8a/libVkLayer_khronos_validation.so")
    }

    ...
}
```

**[2022-07-29]** _Google_ guys confirmed the issue and promised to fix it in the next _Android Studio_ release. The [link](https://issuetracker.google.com/issues/239589378#comment3).
