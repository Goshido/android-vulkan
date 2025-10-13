# Update plan

## Conventions

- ✅ Ready
- ❌ Not ready

## Steps

- ❌ Android Studio
  - ❌ IDE
  - ❌ Wrapper
  - ❌ NDK
  - ❌ CMake
  - ❌ Readme
- ❌ Kotlin
  - ❌ [Kotlin](https://repo.maven.apache.org/maven2/org/jetbrains/kotlin/kotlin-gradle-plugin/)
  - ❌ Kotlin plugin
  - ❌ Readme
- ❌ Gradle
  - ❌ [Binary](https://services.gradle.org/distributions)
  - ❌ Gradle plugin
  - ❌ Readme
- ❌ HTML validator
  - ❌ Update Visual Studio
  - ❌ Update version
  - ❌ Recompile tool
  - ❌ Check tool
  - ❌ Readme
- ❌ 3ds Max plugin
  - ❌ Update Visual Studio
  - ❌ Update version
  - ❌ Recompile plugin
  - ❌ Check plugin
  - ❌ Readme
- ❌ VVL
  - ❌ Binary
    - ❌ _Android_
    - ❌ _Windows_
  - ❌ Check new validation errors
    - ❌ _Android_
    - ❌ _Windows_
  - ❌ Compilation script repo
  - ❌ VVL docs
  - ❌ Readme
    - ❌ Main page
    - ❌ Editor page
- ❌ Lua
  - ❌ Binary
  - ❌ Check
  - ❌ Compilation script repo
  - ❌ Headers
  - ❌ Readme
- ❌ DXC
  - ❌ Update Visual Studio
  - ❌ Binary
  - ❌ Check for new params
  - ❌ Compilation script repo
  - ❌ Shader blobs
  - ❌ External archive
  - ❌ Shader compilation docs
  - ❌ Readme
    - ❌ Main page
    - ❌ Editor page
- ❌ FreeType
  - ❌ Binary
    - ❌ _Android_
    - ❌ _Windows_
  - ❌ Compilation script repo
  - ❌ Headers
  - ❌ Readme
    - ❌ Main page
    - ❌ Editor page
- ❌ Ogg
  - ❌ Binary
  - ❌ Check
  - ❌ Compilation script repo
  - ❌ Headers
  - ❌ Readme
- ❌ Vorbis|Vorbisfile
  - ❌ Binary
  - ❌ Check
  - ❌ Compilation script repo
  - ❌ Headers
  - ❌ Readme
- ❌ STB
  - ❌ Check
  - ❌ Header
  - ❌ Readme
    - ❌ Main page
    - ❌ Editor page
- ❌ Vulkan SDK
  - ❌ Install
  - ❌ Check
  - ❌ Editor page
- ❌ NVIDIA Nsight Graphics
  - ❌ Install
  - ❌ Check
  - ❌ Editor page
- ❌ PIX
  - ❌ Install
  - ❌ Check
  - ❌ Editor page
- ❌ Code checks
- ❌ Update RenderDoc version in documentation
- ❌ Set starting project as PBR
- ❌ Set VSYNC on
- ❌ Remove old binaries
- ❌ Create archive tag with direct link to issue

## Commit messages

Documentation:
- VVL page has been updated
- shader compilation page has been updated
- HTML validator page has been updated
- 3ds Max exporter page has been updated
- RenderDoc page has been updated
- Editor page has been updated
- Release build page has been updated
- build requirements have been updated

Project:
- Android Studio Narwhal 4 Feature Drop | 2025.1.4 support
- Android Studio Gradle Plugin has been updated to 8.13.0
- Android NDK has been updated to 29.0.1420686
- Android SDK Build-Tools updated to 36.1.0
- Android SDK Platform-Tools updated to 36.0.0
- Kotlin has been updated to 2.2.20
- Kotlin Gradle plugin has been updated to 2.2.20
- Gradle has been updated to 9.1.0-bin
- CMake has been updated to 4.1.2
- DirectX Shader Compiler has been updated to 1.8.2505.10149, 0bf8434bc3b57a0b99477162dfe54673d9b5153b
- SPIR-V shader blobs have been recompiled
- FreeType 2.14.1 has been updated to 4334f009e7d20789cc7ee1224290ea1e22a17b5b
- VVL has been updated to 1.4.329, 62d79257ac9b93ba3f6fa7507fb172cb9cf8e7ff
- Ogg 1.3.6 has been updated to 0288fadac3ac62d453409dfc83e9c4ab617d2472
- Vorbis 1.3.7 has been updated to 851cce991da34adf5e1f3132588683758a6369ec
- Vorbisfile 1.3.7 has been updated to 851cce991da34adf5e1f3132588683758a6369ec
- Lua 5.5.0 has been updated to 9ea06e61f20ae34974226074fc6123dbb54a07c2
- stb_image has been updated to 2.30, fede005abaf93d9d7f3a679d1999b2db341b360f
- PowerShell has been updated to 7.5.3
- Shader model has been changed to 6_9

3rd-party:
- FreeType headers have been updated
- FreeType binary has been updated
- VVL binary has been updated
- Lua header files have been updated
- Lua binary has been updated
- gradlew has been updated
- libogg has been recompiled
- libogg header files have been updated
- libvorbis and libvorbisfile have been recompiled
- libvorbis and libvorbisfile header files have been updated

Editor:
- 3ds Max exporter has been updated to 1.0.1.10
- Visual Studio 2022 Community 17.14.7 support
- Windows 11 SDK has been changed to 10.0.26100.3916
- MSVC v143 - VS 2022 C++ x64/x86 build tools has been updated to v14.44-17.14
- Vulkan SDK has been changed to 1.4.328.1
- NVIDIA Nsight Graphics 2025.4.1.0 (build 36508989) support
- NVIDIA Aftermath support
- PIX has been updated to 2409.23
* RenderDoc 1.40 support
- WinPixEventRuntime.(dll|lib) has been updated to 1.0.240308001

HTML validator:
- HTML validator has been updated to 1.0.1.15
- Windows 11 support
- Visual Studio 2022 Community 17.14.16 support
- Windows 11 SDK has been changed to 10.0.26100.4654
- MSVC v143 - VS 2022 C++ x64/x86 build tools has been updated to v14.44-17.14
- CMake has been updated to 4.1.2

3ds Max exporter:
- 3ds Max exporter has been updated to 1.0.1.11
- Visual Studio 2022 Community 17.14.16 support
- Windows 11 SDK has been changed to 10.0.26100.4654
- MSVC v143 - VS 2022 C++ x64/x86 build tools has been updated to v14.44-17.14
