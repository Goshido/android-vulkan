# Building _Vulkan_ validation layers for _Android OS_

## Brief

Starting from _Android NDK_ `23.0.7599858` the _Vulkan_ validation layers have been removed (see [source](https://github.com/android/ndk/wiki/Changelog-r23)). The official _Khronos_ [repo](https://github.com/KhronosGroup/Vulkan-ValidationLayers) does not provide enough information about building `libVkLayer_khronos_validation.so` from scratch for _Android OS_. So in this document essential steps for building will be covered.

## Compatible version

The manual is based on `6eaf97e9366755822923147c94e0ee1760a393d5` commit of the [_Vulkan-ValidationLayers_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) repo. The manual is primary aimed for _Windows OS_ users.

## Requirements

- _Windows 11_
- _Android NDK 26.3.11579264_
- _CMake 3.22.1_
- _Ninja 1.10.2_
- _Python 3.11.1_

## Building

First of all clone the [_Vulkan-ValidationLayers_](https://github.com/KhronosGroup/Vulkan-ValidationLayers) repo. Next step is to switch to the `master` branch.

Then you have to prepare building utils for _Vulkan-ValidationLayers_ project. It is [_Python 3.7+_](https://www.python.org/).

After installing the _Python_ you have to disable _Windows 11_ aliases for python apps.

<img src="./images/python-windows-aliases.png" width="629" />

It's needed because bootstrap scripts of the _Vulkan-ValidationLayers_ project are using `python3` calls. So the next obvious step is to create symbolic link `python3.exe` which will be connected with `python.exe`. Yes. It's possible on _Windows OS_. Run _cmd.exe_ with administrative rights and call

```batch
cd <python directory>
mklink python3.exe python.exe
```

### Building `libVkLayer_khronos_validation.so`

Starting from _VVL_ `eca34aae4cc04eb32035a7b1770a276933f37327` building process is significantly simplified:

```PowerShell
Clear-Host

# Preparing sources...

$ndk = "D:/Programs/Android/Sdk/ndk/26.3.11579264"

$androidVulkanDir = "D:/Development/android-vulkan"
$vvlDir = "D:/Development/Vulkan-ValidationLayers"

$abi = "arm64-v8a"
$androidAPI = 30
$buildThreads = 16

$buildDir = "${vvlDir}/build-android/obj/${abi}"
$libDir = "third-party/jniLibs/${abi}"

# Cleaning repo...

Set-Location "${vvlDir}"
git clean -d -fx -f

# Making project files...

$api = "android-${androidAPI}"

cmake                                                                       `
    -S .                                                                    `
    -B "${buildDir}"                                                        `
    -G Ninja                                                                `
    -D ANDROID_PLATFORM=$api                                                `
    -D ANDROID_USE_LEGACY_TOOLCHAIN_FILE=NO                                 `
    -D CMAKE_ANDROID_ARCH_ABI=${abi}                                        `
    -D CMAKE_ANDROID_STL_TYPE=c++_static                                    `
    -D CMAKE_INSTALL_LIBDIR="${libDir}"                                     `
    -D CMAKE_TOOLCHAIN_FILE="${ndk}/build/cmake/android.toolchain.cmake"    `
    -D CMAKE_BUILD_TYPE=Release                                             `
    -D UPDATE_DEPS=ON                                                       `
    -D UPDATE_DEPS_DIR="${buildDir}"

# Building projects...

$threads = "-j${buidThreads}"

cmake                                                                       `
    --build "${buildDir}"                                                   `
    $threads

# Stripping binaries and copying them into android-vulkan project...

cmake                                                                       `
    --install "${buildDir}"                                                 `
    --strip                                                                 `
    --prefix "${androidVulkanDir}"

Write-Host "Done"

```

That's all!

### Reported issues

Name | Link | Status
--- | --- | ---
Incorrect validation of the `vkGetPhysicalDeviceSurfaceFormatsKHR` | [#3251](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/3251) | ✔️ Fixed
Incorrect de-initialization and `VK_EXT_debug_report` | [#3327](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/3327) | ☄️ Driver bug
Input attachment descriptor image view is not a subpass input attachment | [#4555](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4555) | ✔️ Fixed
master build broken for some time | [#5014](https://github.com/KhronosGroup/SPIRV-Tools/issues/5014) | ✔️ Fixed
Broken Android build | [#4947](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/4947) | ✔️ Fixed
`vkCmdNextSubpass` validation issue | [#5853](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5853) | 🛡️ Not an issue
Full screen triangle `VkPipeline` issue | [#7636](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/7636) | ✔️ Fixed
First `vkGetSwapchainImagesKHR` false positive | [#8138](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8138) | ✔️ Fixed
No resource type and name in error message | [#8139](https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8139) | ⚠️ Submitted
