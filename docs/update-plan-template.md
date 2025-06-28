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
- build requirements have been updated

Project:
- Android Studio Narwhal | 2025.1.1 support
- Android Studio Gradle Plugin has been updated to 8.11.0
- Android NDK has been updated to 28.1.13356709
- Android SDK Build-Tools updated to 36.0.0
- Android SDK Platform-Tools updated to 36.0.0
- Kotlin has been updated to 2.2.0
- Kotlin Gradle plugin has been updated to 2.2.0
- Gradle has been updated to 8.14.2-bin
- CMake has been updated to 4.0.2
- DirectX Shader Compiler has been updated to 1.8.2505.10062, d39324e0635130e834a68e33b0c603cf5fc9fb4f
- SPIR-V shader blobs have been recompiled
- FreeType 2.13.3 has been updated to 58be4879c5d3840315f037dca44e92384113f8f9
- VVL has been updated to 1.4.320, 7d29258f5e5bb765057929b217d9a9662315e610
- Ogg 1.3.6 has been updated to fe20a3ed04b9e4de8d2b4c753077d9a7c2a7e588
- Vorbis 1.3.7 has been updated to 43bbff0141028e58d476c1d5fd45dd5573db576d
- Vorbisfile 1.3.7 has been updated to 43bbff0141028e58d476c1d5fd45dd5573db576d
- Lua 5.5.0 has been updated to cfce6f4b20afe85ede2182b3df3ab2bfcdb0e692
- stb_image has been updated to 2.30, f58f558c120e9b32c217290b80bad1a0729fbb2c
- PowerShell has been updated to 7.5.2
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
- PIX has been updated to 2409.23
- WinPixEventRuntime.(dll|lib) has been updated to 1.0.240308001

HTML validator:
- HTML validator has been updated to 1.0.1.14
- Windows 11 support
- Visual Studio 2022 Community 17.14.7 support
- Windows 11 SDK has been changed to 10.0.26100.3916
- MSVC v143 - VS 2022 C++ x64/x86 build tools has been updated to v14.44-17.14
- CMake has been updated to 4.0.2

3ds Max exporter:
- 3ds Max exporter has been updated to 1.0.1.10
- Visual Studio 2022 Community 17.14.7 support
- Windows 11 SDK has been changed to 10.0.26100.3916
- MSVC v143 - VS 2022 C++ x64/x86 build tools has been updated to v14.44-17.14
