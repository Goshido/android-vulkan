:: Must be set
:: NDK_DIR - directory where Android NDK is installed
::      for example D:\Development\android-sdk\ndk\23.0.7599858
::
:: VK_LAYER_DIR - directory where Vulkan validation layer source code is located for Android
::      for example D:\Development\Vulkan-ValidationLayers\build-android

@echo off

:: Copying CMakeLists.txt to Android build directory
xcopy /s/y "..\CMakeLists.txt" "%VK_LAYER_DIR%"

pushd %cd%

:: Creating Ninja files for Android 11|arm64-v8a platform...
cmake -H%VK_LAYER_DIR% -B%VK_LAYER_DIR%/build/arm64-v8a -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-30 -DANDROID_NDK=%NDK_DIR% -DANDROID_STL=c++_static -DCMAKE_TOOLCHAIN_FILE=%NDK_DIR%/build/cmake/android.toolchain.cmake -G "Ninja Multi-Config"

:: Building release version of the libVkLayer_khronos_validation.so
cd %VK_LAYER_DIR%/build/arm64-v8a
ninja -f build-Release.ninja

:: Stripping the dynamic library
mkdir "Release/stripped" 2> nul
"%NDK_DIR%/toolchains/llvm/prebuilt/windows-x86_64/bin/llvm-strip.exe" -s -o Release/stripped/libVkLayer_khronos_validation.so Release/libVkLayer_khronos_validation.so

echo Stripped libVkLayer_khronos_validation.so is located in %VK_LAYER_DIR%/build/arm64-v8a/Release/stripped
echo(

:: Restoring current directory
popd

:: Pausing
pause

@echo on
