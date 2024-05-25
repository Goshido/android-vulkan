# Update plan

## Conventions

- ✅ Ready
- ❌ Not ready

## Steps

- ✅ Android Studio
  - ✅ IDE
  - ✅ Wrapper
  - ✅ NDK
  - ✅ Readme
- ✅ Kotlin
  - ✅ [Kotlin](https://repo.maven.apache.org/maven2/org/jetbrains/kotlin/kotlin-gradle-plugin/)
  - ✅ Kotlin plugin
  - ✅ Readme
- ✅ Gradle
  - ✅ [Binary](https://services.gradle.org/distributions)
  - ✅ Gradle plugin
  - ✅ Readme
- ✅ HTML validator
  - ✅ Update Visual Studio
  - ✅ Update version
  - ✅ Recompile tool
  - ✅ Check tool
  - ✅ Readme
- ✅ 3ds Max plugin
  - ✅ Update Visual Studio
  - ✅ Update version
  - ✅ Recompile plugin
  - ✅ Check plugin
  - ✅ Readme
- ✅ VVL
  - ✅ Binary
  - ✅ Check new validation errors
  - ✅ Compilation script repo
  - ✅ VVL docs
  - ✅ Readme
- ✅ Lua
  - ✅ Binary
  - ✅ Compilation script repo
  - ✅ Header
  - ✅ Readme
- ✅ DXC
  - ✅ Update Visual Studio
  - ✅ Binary
  - ✅ Check for new params
  - ✅ Compilation script repo
  - ✅ Shader blobs
  - ✅ External archive
  - ✅ Shader compilation docs
  - ✅ Readme
- ✅ FreeType
  - ✅ Binary
  - ✅ Compilation script repo
  - ✅ Header
  - ✅ Readme
- ✅ Ogg
  - ✅ Binary
  - ✅ Compilation script repo
  - ✅ Header
  - ✅ Readme
- ✅ Vorbis|Vorbisfile
  - ✅ Binary
  - ✅ Compilation script repo
  - ✅ Header
  - ✅ Readme
- ✅ STB
  - ✅ Header
  - ✅ Readme
- ✅ Code checks
- ✅ Update RenderDoc version in documentation
- ✅ Set starting project as PBR
- ❌ Create archive tag with direct link to issue

## Commit messages

Documentation:
- VVL page has been updated
- shader compilation page has been updated
- HTML validator page has been updated
- 3ds Max exporter page has been updated
- build requirements have been updated

Project:
- Android Studio Jellyfish | 2023.3.1 Patch 1 support
- Android Studio Gradle Plugin has been updated to 8.4.1
- Android NDK has been updated to 26.3.11579264
- Android SDK Platform-Tools updated to 35.0.1
- Kotlin has been updated to 2.0.0
- Kotlin Gradle plugin has been updated to 2.0.0
- Gradle has been updated to 8.7-bin
- DirectX Shader Compiler has been updated to 1.8.2405.10035, 128e6ce2be8f09539c97cf28d6066d9a6b32b15c
- SPIR-V shader blobs have been recompiled (no changes)
- FreeType has been updated to 2.13.2, b6dbbd963097afd9f90cc02f1c6c6d2b98ca4fd4
- VVL has been updated to 1.3.285, df717cc614855e2b81660798da349043bb37472a
- Lua 5.5.0 has been updated to 262dc5729a28b2bad0b6413d4eab2290d14395cf
- stb_image 2.29 has been updated to ae721c50eaf761660b4f90cc590453cdb0c2acd0 NO NO NO

Common:
- AABB overlap test with Neon

3rd-party:
- FreeType headers have been updated
- FreeType binary has been updated
- Lua header files have been updated
- Lua binary has been updated
- VVL binary has been updated
- gradlew has been updated
- libogg has been recompiled
- libogg header files have been updated
- libvorbis and libvorbisfile have been recompiled
- libvorbis and libvorbisfile header files have been updated

tools:
- HTML validator has been updated to 1.0.1.8
- 3ds Max exporter has been updated to 1.0.1.4
