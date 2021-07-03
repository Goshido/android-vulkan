cmake_minimum_required ( VERSION 3.18.1 )
set ( CMAKE_CXX_STANDARD 17 )
project ( android-vulkan )

# Enable link time optimization for release build via native flag.
# Note there is CMAKE_INTERPROCEDURAL_OPTIMZATION_RELEASE built-in variable but it works not reliably with
# NDK 22.0.7026061 at link time (linking errors on Linux).
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -flto=thin")

# Android native activity glue

add_library ( native_app_glue
    STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

target_include_directories ( native_app_glue
    PUBLIC
    ${ANDROID_NDK}/sources/android/native_app_glue
)

target_compile_options ( native_app_glue PRIVATE
    -fno-exceptions
    -fno-rtti
)

set_target_properties ( native_app_glue PROPERTIES
    CXX_VISIBILITY_PRESET hidden
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

target_compile_options ( vulkan-wrapper PRIVATE
    -fno-exceptions
    -fno-rtti
)

set_target_properties ( vulkan-wrapper PROPERTIES
    CXX_VISIBILITY_PRESET hidden
)

# log support
find_library ( log-lib
    log
)

# Main app

add_library ( android-vulkan
    SHARED
    app/src/main/cpp/sources/contact_detector.cpp
    app/src/main/cpp/sources/contact_manager.cpp
    app/src/main/cpp/sources/core.cpp
    app/src/main/cpp/sources/cyrus_beck.cpp
    app/src/main/cpp/sources/epa.cpp
    app/src/main/cpp/sources/file.cpp
    app/src/main/cpp/sources/gamepad.cpp
    app/src/main/cpp/sources/gjk.cpp
    app/src/main/cpp/sources/global_force_gravity.cpp
    app/src/main/cpp/sources/half_types.cpp
    app/src/main/cpp/sources/key_bind.cpp
    app/src/main/cpp/sources/ktx_media_container.cpp
    app/src/main/cpp/sources/location_solver.cpp
    app/src/main/cpp/sources/logger.cpp
    app/src/main/cpp/sources/main.cpp
    app/src/main/cpp/sources/mesh_geometry.cpp
    app/src/main/cpp/sources/physics.cpp
    app/src/main/cpp/sources/renderer.cpp
    app/src/main/cpp/sources/rigid_body.cpp
    app/src/main/cpp/sources/shape.cpp
    app/src/main/cpp/sources/shape_box.cpp
    app/src/main/cpp/sources/simplex.cpp
    app/src/main/cpp/sources/stick.cpp
    app/src/main/cpp/sources/sutherland_hodgman.cpp
    app/src/main/cpp/sources/texture_cube.cpp
    app/src/main/cpp/sources/texture2D.cpp
    app/src/main/cpp/sources/trigger.cpp
    app/src/main/cpp/sources/uniform_buffer.cpp
    app/src/main/cpp/sources/vulkan_utils.cpp
    app/src/main/cpp/sources/GXCommon/GXMath.cpp
    app/src/main/cpp/sources/GXCommon/Intrinsics/GXMathNeon.cpp
    app/src/main/cpp/sources/GXCommon/Vulkan/GXMathBackend.cpp
    app/src/main/cpp/sources/mandelbrot/mandelbrot_analytic_color.cpp
    app/src/main/cpp/sources/mandelbrot/mandelbrot_base.cpp
    app/src/main/cpp/sources/mandelbrot/mandelbrot_lut_color.cpp
    app/src/main/cpp/sources/pbr/camera.cpp
    app/src/main/cpp/sources/pbr/component.cpp
    app/src/main/cpp/sources/pbr/cube_map_manager.cpp
    app/src/main/cpp/sources/pbr/gbuffer.cpp
    app/src/main/cpp/sources/pbr/geometry_pass.cpp
    app/src/main/cpp/sources/pbr/light.cpp
    app/src/main/cpp/sources/pbr/light_pass.cpp
    app/src/main/cpp/sources/pbr/lightup_common_descriptor_set.cpp
    app/src/main/cpp/sources/pbr/lightup_common_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/light_lightup_base_program.cpp
    app/src/main/cpp/sources/pbr/light_volume.cpp
    app/src/main/cpp/sources/pbr/light_volume_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/light_volume_program.cpp
    app/src/main/cpp/sources/pbr/material.cpp
    app/src/main/cpp/sources/pbr/material_manager.cpp
    app/src/main/cpp/sources/pbr/mesh_group.cpp
    app/src/main/cpp/sources/pbr/mesh_manager.cpp
    app/src/main/cpp/sources/pbr/opaque_call.cpp
    app/src/main/cpp/sources/pbr/opaque_material.cpp
    app/src/main/cpp/sources/pbr/opaque_program.cpp
    app/src/main/cpp/sources/pbr/opaque_instance_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/opaque_texture_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/pbr_game.cpp
    app/src/main/cpp/sources/pbr/point_light.cpp
    app/src/main/cpp/sources/pbr/point_light_component.cpp
    app/src/main/cpp/sources/pbr/point_light_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/point_light_lightup.cpp
    app/src/main/cpp/sources/pbr/point_light_lightup_program.cpp
    app/src/main/cpp/sources/pbr/point_light_pass.cpp
    app/src/main/cpp/sources/pbr/point_light_shadowmap_generator_program.cpp
    app/src/main/cpp/sources/pbr/present_pass.cpp
    app/src/main/cpp/sources/pbr/program.cpp
    app/src/main/cpp/sources/pbr/reflection_component.cpp
    app/src/main/cpp/sources/pbr/reflection_global_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/reflection_global_pass.cpp
    app/src/main/cpp/sources/pbr/reflection_global_program.cpp
    app/src/main/cpp/sources/pbr/reflection_local_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/reflection_local_pass.cpp
    app/src/main/cpp/sources/pbr/reflection_local_program.cpp
    app/src/main/cpp/sources/pbr/reflection_probe.cpp
    app/src/main/cpp/sources/pbr/reflection_probe_global.cpp
    app/src/main/cpp/sources/pbr/reflection_probe_local.cpp
    app/src/main/cpp/sources/pbr/render_session.cpp
    app/src/main/cpp/sources/pbr/render_session_stats.cpp
    app/src/main/cpp/sources/pbr/sampler.cpp
    app/src/main/cpp/sources/pbr/sampler_manager.cpp
    app/src/main/cpp/sources/pbr/static_mesh_component.cpp
    app/src/main/cpp/sources/pbr/stub_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/texture_present_descriptor_set_layout.cpp
    app/src/main/cpp/sources/pbr/texture_present_program.cpp
    app/src/main/cpp/sources/pbr/uniform_buffer_pool.cpp
    app/src/main/cpp/sources/rainbow/rainbow.cpp
    app/src/main/cpp/sources/rotating_mesh/game.cpp
    app/src/main/cpp/sources/rotating_mesh/game_analytic.cpp
    app/src/main/cpp/sources/rotating_mesh/game_lut.cpp
)

# ARM Neon support: All ARMv8-based devices support Neon. So no any additional compile flags need to specify.
# See https://developer.android.com/ndk/guides/cpu-arm-neon#cmake_2

target_include_directories ( android-vulkan
    PRIVATE
    ${ANDROID_NDK}/sources/third_party/vulkan/src/common
    app/src/main/cpp/include
)

# Disable libandroid-vulkan.so symbol publishing
set_target_properties ( android-vulkan PROPERTIES
    CXX_VISIBILITY_PRESET hidden
)

# Treat compile warnings as errors
target_compile_options ( android-vulkan PRIVATE
    -fno-exceptions
    -fno-rtti
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

# Exporting native app glue entry point.
# https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html
set (
    CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate"
)

target_link_libraries ( android-vulkan
    android
    ${log-lib}
    native_app_glue
    vulkan-wrapper
)
