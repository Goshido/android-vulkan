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
- ❌ PowerShell
  - ❌ Binary
  - ❌ Readme main
  - ❌ Readme editor
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
  - ❌ Check plugin (animation 0 - 33 frames)
  - ❌ Readme
- ❌ VVL
  - ❌ Binary
    - ❌ _Android_
    - ❌ _Windows_
  - ❌ Check for option changes
  - ❌ Check new validation errors
    - ❌ _Android_
    - ❌ _Windows_
  - ❌ Check if some issues have been fixed
  - ❌ Compilation script repo
  - ❌ VVL docs (NDK version)
  - ❌ Readme
    - ❌ Main page
    - ❌ Editor page
- ❌ Lua
  - ❌ Binary
  - ❌ Check
  - ❌ Compilation script repo
  - ❌ Headers
  - ❌ Readme
    - ❌ Main page
    - ❌ Lua scripting frontend
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
- ❌ FreeType (use main branch now)
  - ❌ Check new compile options
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
- ❌ Update RenderDoc version in documentation (fix DXC version lookup)
- ❌ Set starting project as PBR
- ❌ Set VSYNC on
- ❌ Remove old binaries
- ❌ Exclude hlsl.cpp from compilation
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
- Android Studio Quail 4 | 2026.1.4 Patch 1
- Android Studio Gradle Plugin has been updated to 9.4.1
- Android NDK has been updated to 30.0.16248370
- Android SDK Build-Tools updated to 37.0.0
- Android SDK Platform-Tools updated to 37.0.1
- Kotlin has been updated to 2.4.20
- Kotlin Gradle plugin has been updated to 2.4.20
- Gradle has been updated to 9.6.0-bin
- CMake has been updated to 4.1.2
- DirectX Shader Compiler has been updated to 1.10.2609.10012, 717b24d7a487efb555e976104a263f93f181fd44
- SPIR-V shader blobs have been recompiled
- FreeType has been updated to 2.14.3 d333439633039de426f943f28a2926c7f97b5ae5
- VVL has been updated to 1.4.364, 4315e7fd4500673692174ed4fee92277a200a7c5
- Ogg 1.3.6 has been updated to 06a5e0262cdc28aa4ae6797627a783b5010440f0
- Vorbis 1.3.7 has been updated to 1b75110b5a2754ba1931d82dd83cb822b266a21d
- Vorbisfile 1.3.7 has been updated to 1b75110b5a2754ba1931d82dd83cb822b266a21d
- Lua has been updated to 5.5.1 0b29f408433e92953cc72b1d3e06c7ac8139e439
- stb_image has been updated to 2.30, 2c980bb59875b0d32144a71867fbdebb2f77cd20
- PowerShell has been updated to 7.6.6
- Shader model has been changed to 6_11
- RenderDoc 1.46 support

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
- Visual Studio 2026 Community 18.10.2 support
- Windows 11 SDK has been changed to 10.0.28000.2526
- Using MSVC Build tools v14.51 for x64/x86
- Vulkan SDK has been changed to 1.4.357.0
- NVIDIA Nsight Graphics 2026.3.1.0 (build 38722833) support
- NVIDIA Aftermath support
- PIX has been updated to 2603.25

HTML validator:
- HTML validator has been updated to 1.0.1.18
- Visual Studio 2026 Community 18.10.2 support
- Windows 11 SDK has been updated to 10.0.28000.2526
- Using MSVC Build tools v14.51 for x64/x86
- CMake has been updated to 4.2

3ds Max exporter:
- 3ds Max exporter has been updated to 1.0.1.14
- Visual Studio 2026 Community 18.10.2 support
- Windows 11 SDK has been updated to 10.0.28000.2526
- Using MSVC Build tools v14.51 for x64/x86
