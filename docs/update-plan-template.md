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
  - ❌ [_MikkTSpace_](https://github.com/mmikk/MikkTSpace)
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
- Android Studio Otter | 2025.2.1 support
- Android Studio Gradle Plugin has been updated to 8.13.1
- Android NDK has been updated to 29.0.1420686
- Android SDK Build-Tools updated to 36.1.0
- Android SDK Platform-Tools updated to 36.0.0
- Kotlin has been updated to 2.2.21
- Kotlin Gradle plugin has been updated to 2.2.21
- Gradle has been updated to 9.2.0-bin
- CMake has been updated to 4.1.2
- DirectX Shader Compiler has been updated to 1.8.2505.10178, b1cf2cad8f19f2ce733bd108e63485b33fbd4774
- SPIR-V shader blobs have been recompiled
- FreeType 2.14.1 has been updated to fc9cc5038e05edceec3d0f605415540ac76163e9
- VVL has been updated to 1.4.332, fc24b1981d8e11ed35ee2af0d9f43b92285e38a8
- Ogg 1.3.6 has been updated to 0288fadac3ac62d453409dfc83e9c4ab617d2472
- Vorbis 1.3.7 has been updated to 851cce991da34adf5e1f3132588683758a6369ec
- Vorbisfile 1.3.7 has been updated to 851cce991da34adf5e1f3132588683758a6369ec
- Lua 5.5.0 has been updated to fca974486d12aa29bb6d731fdb5b25055157ece8
- stb_image has been updated to 2.30, f1c79c02822848a9bed4315b12c8c8f3761e1296
- PowerShell has been updated to 7.5.3
- Shader model has been changed to 6_10
- Switch to C++23

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
- Visual Studio 2026 Community 18.0.0 support
- Windows 11 SDK has been changed to 10.0.26100.6901
- MSVC Build Tools for x64/x86 has been updated to v14.50
- Vulkan SDK has been changed to 1.4.328.1
- NVIDIA Nsight Graphics 2025.4.1.0 (build 36508989) support
- NVIDIA Aftermath support
- PIX has been updated to 2409.23
- RenderDoc 1.41 support
- WinPixEventRuntime.(dll|lib) has been updated to 1.0.240308001

HTML validator:
- HTML validator has been updated to 1.0.1.17
- Visual Studio 2026 Community 18.0.0 support
- Windows 11 SDK has been updated to 10.0.26100.6901
- MSVC Build Tools for x64/x86 has been updated to v14.50
- CMake has been updated to 4.2

3ds Max exporter:
- 3ds Max exporter has been updated to 1.0.1.13
- Visual Studio 2026 Community 18.0.0 support
- Windows 11 SDK has been updated to 10.0.26100.6901
- MSVC Build Tools for x64/x86 has been updated to v14.50
