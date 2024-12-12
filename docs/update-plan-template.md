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
- ❌ Lua
  - ❌ Binary
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
- ❌ FreeType
  - ❌ Binary
    - ❌ _Android_
    - ❌ _Windows_
  - ❌ Compilation script repo
  - ❌ Headers
  - ❌ Readme
- ❌ Ogg
  - ❌ Binary
  - ❌ Compilation script repo
  - ❌ Headers
  - ❌ Readme
- ❌ Vorbis|Vorbisfile
  - ❌ Binary
  - ❌ Compilation script repo
  - ❌ Headers
  - ❌ Readme
- ❌ STB
  - ❌ Header
  - ❌ Readme
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
- build requirements have been updated

Project:
- Android Studio Ladybug | 2024.2.1 Patch 3 support
- Android Studio Gradle Plugin has been updated to 8.7.3
- Android NDK has been updated to 27.2.12479018
- Android SDK Platform-Tools updated to 35.0.2
- Kotlin has been updated to 2.1.0
- Kotlin Gradle plugin has been updated to 2.1.0
- Gradle has been updated to 8.11.1-bin
- CMake has been updated to 3.31.1
- DirectX Shader Compiler has been updated to 1.8.2407.10120, d39324e0635130e834a68e33b0c603cf5fc9fb4f
- SPIR-V shader blobs have been recompiled (no changes)
- FreeType 2.13.3 has been updated to 59320b2d3c2584ac01914ed0deff64bcc8fb23b2
- VVL has been updated to 1.4.303, e1b6492a0b1491f82f3f6c5a2054b7bdc711ad5f
- Ogg 1.3.5 has been updated to 7cf42ea17aef7bc1b7b21af70724840a96c2e7d0
- Vorbis 1.3.7 has been updated to bb4047de4c05712bf1fd49b9584c360b8e4e0adf
- Vorbisfile 1.3.7 has been updated to bb4047de4c05712bf1fd49b9584c360b8e4e0adf
- Lua 5.5.0 has been updated to 2fccd3c4b9043a77b0be2da74c8c7ffe31ce28c7
- stb_image has been updated to 2.30, f7f20f39fe4f206c6f19e26ebfef7b261ee59ee4

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

HTML validator:
- HTML validator has been updated to 1.0.1.12
- Windows 11 support
- Visual Studio 2022 Community 17.12.3 support
- Windows 11 SDK has been changed to 10.0.26100
- MSVC v143 - VS 2022 C++ x64/x86 build tools has been updated to v14.42-17.12
- CMake has been updated to 3.31.1

3ds Max exporter:
- 3ds Max exporter has been updated to 1.0.1.9
- Windows 11 support
- Visual Studio 2022 Community 17.12.3 support
- Windows 11 SDK has been changed to 10.0.26100
