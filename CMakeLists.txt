cmake_minimum_required ( VERSION 3.10.2 )
set ( CMAKE_CXX_STANDARD 17 )

# Android native activity glue

add_library ( native_app_glue
    STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

target_include_directories ( native_app_glue
    PUBLIC
    ${ANDROID_NDK}/sources/android/native_app_glue
)

# Android NDK Vulkan wrapper

add_library ( vulkan-wrapper
    STATIC
    ${ANDROID_NDK}/sources/third_party/vulkan/src/common/vulkan_wrapper.cpp
)

target_include_directories ( vulkan-wrapper
    PUBLIC
    ${ANDROID_NDK}/sources/third_party/vulkan/src/include
)

# Control macro for ability to create Vulkan surface for Android.
# See vulkan_wrapper.h
# See Vulkan spec - 32.2.1. Android Platform
# https://vulkan.lunarg.com/doc/view/1.1.108.0/mac/vkspec.html
target_compile_definitions( vulkan-wrapper
    PUBLIC
    VK_USE_PLATFORM_ANDROID_KHR
)

# log support
find_library ( log-lib
    log
)

# Main app

set (
    CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate"
)

add_library ( android-vulkan
    SHARED
    app/src/main/cpp/sources/core.cpp
    app/src/main/cpp/sources/file.cpp
    app/src/main/cpp/sources/gamepad.cpp
    app/src/main/cpp/sources/key_bind.cpp
    app/src/main/cpp/sources/logger.cpp
    app/src/main/cpp/sources/main.cpp
    app/src/main/cpp/sources/mesh_geometry.cpp
    app/src/main/cpp/sources/renderer.cpp
    app/src/main/cpp/sources/stick.cpp
    app/src/main/cpp/sources/texture2D.cpp
    app/src/main/cpp/sources/trigger.cpp
    app/src/main/cpp/sources/vulkan_utils.cpp
    app/src/main/cpp/sources/GXCommon/GXMath.cpp
    app/src/main/cpp/sources/GXCommon/Vulkan/GXMathBackend.cpp
    app/src/main/cpp/sources/mandelbrot/mandelbrot_analytic_color.cpp
    app/src/main/cpp/sources/mandelbrot/mandelbrot_base.cpp
    app/src/main/cpp/sources/mandelbrot/mandelbrot_lut_color.cpp
    app/src/main/cpp/sources/pbr/camera.cpp
    app/src/main/cpp/sources/pbr/gbuffer.cpp
    app/src/main/cpp/sources/pbr/material.cpp
    app/src/main/cpp/sources/pbr/mesh_group.cpp
    app/src/main/cpp/sources/pbr/opaque_call.cpp
    app/src/main/cpp/sources/pbr/opaque_material.cpp
    app/src/main/cpp/sources/pbr/opaque_program.cpp
    app/src/main/cpp/sources/pbr/opaque_batch_program.cpp
    app/src/main/cpp/sources/pbr/opaque_instance_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/opaque_texture_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/pbr_game.cpp
    app/src/main/cpp/sources/pbr/program.cpp
    app/src/main/cpp/sources/pbr/program_resource.cpp
    app/src/main/cpp/sources/pbr/render_session.cpp
    app/src/main/cpp/sources/pbr/sampler.cpp
    app/src/main/cpp/sources/pbr/texture_present_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/texture_present_program.cpp
    app/src/main/cpp/sources/pbr/uniform_buffer_pool.cpp
    app/src/main/cpp/sources/rainbow/rainbow.cpp
    app/src/main/cpp/sources/rotating_mesh/game.cpp
    app/src/main/cpp/sources/rotating_mesh/game_analytic.cpp
    app/src/main/cpp/sources/rotating_mesh/game_lut.cpp

    app/src/main/cpp/sources/rotating_mesh/uniform_buffer.cpp
)

target_include_directories ( android-vulkan
    PRIVATE
    ${ANDROID_NDK}/sources/third_party/vulkan/src/common
    app/src/main/cpp/include
)

# Treat compile warnings as errors
target_compile_options ( android-vulkan PRIVATE
    -Werror
    -Wall
    -Wextra
    -Wshadow
)

# See docs/preprocessor-macros.md
target_compile_definitions ( android-vulkan
    PRIVATE
    -DANDROID_NATIVE_MODE_PORTRAIT
    -DANDROID_VULKAN_DEBUG
    -DANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS
    -DANDROID_VULKAN_STRICT_MODE
)

target_link_libraries ( android-vulkan
    android
    ${log-lib}
    native_app_glue
    vulkan-wrapper
)
