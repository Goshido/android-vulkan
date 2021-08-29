cmake_minimum_required ( VERSION 3.18.1 )
set ( CMAKE_CXX_STANDARD 11 )
project ( Vulkan-validation-layers )

# Enable link time optimization for release build via native flag.
# Note there is CMAKE_INTERPROCEDURAL_OPTIMZATION_RELEASE built-in variable but it works not reliably with
# NDK 22.0.7026061 at link time (linking errors on Linux).
string ( APPEND CMAKE_CXX_FLAGS_RELEASE " -flto=thin" )

# layer_utils static library

add_library ( layer_utils
    STATIC
    ../layers/vk_layer_config.cpp
    ../layers/vk_layer_extension_utils.cpp
    ../layers/vk_layer_utils.cpp
    ../layers/vk_format_utils.cpp
)

target_include_directories ( layer_utils
    PRIVATE
    Vulkan-Headers/include
    ../layers
    ../layers/generated
    robin-hood-hashing/src/include
)

target_compile_definitions ( layer_utils
    PRIVATE
    -DVK_ENABLE_BETA_EXTENSIONS
    -DVK_USE_PLATFORM_ANDROID_KHR
    -DVK_PROTOTYPES
    -DUSE_ROBIN_HOOD_HASHING
)

target_compile_options ( layer_utils PRIVATE
    -std=c++11
    -Wall
    -Werror
    -Wno-unused-function
    -Wno-unused-const-variable
)

set_target_properties ( layer_utils PROPERTIES
    CXX_VISIBILITY_PRESET hidden
)

# Finding Android log library

find_library ( log-lib
    log
)

# VkLayer_khronos_validation library

add_library ( VkLayer_khronos_validation
    SHARED
    ../layers/state_tracker.cpp
    ../layers/device_memory_state.cpp
    ../layers/cmd_buffer_state.cpp
    ../layers/image_state.cpp
    ../layers/pipeline_state.cpp
    ../layers/render_pass_state.cpp
    ../layers/core_validation.cpp
    ../layers/drawdispatch.cpp
    ../layers/descriptor_sets.cpp
    ../layers/buffer_validation.cpp
    ../layers/shader_module.cpp
    ../layers/shader_validation.cpp
    ../layers/generated/spirv_validation_helper.cpp
    ../layers/generated/command_validation.cpp
    ../layers/gpu_validation.cpp
    ../layers/gpu_utils.cpp
    ../layers/debug_printf.cpp
    ../layers/best_practices_utils.cpp
    ../layers/generated/best_practices.cpp
    ../layers/sync_utils.cpp
    ../layers/sync_vuid_maps.cpp
    ../layers/core_error_location.cpp
    ../layers/generated/synchronization_validation_types.cpp
    ../layers/synchronization_validation.cpp
    ../layers/convert_to_renderpass2.cpp
    ../layers/generated/layer_chassis_dispatch.cpp
    ../layers/generated/chassis.cpp
    ../layers/layer_options.cpp
    ../layers/xxhash.c
    ../layers/generated/parameter_validation.cpp
    ../layers/parameter_validation_utils.cpp
    ../layers/generated/object_tracker.cpp
    ../layers/object_tracker_utils.cpp
    ../layers/generated/thread_safety.cpp
    ../layers/generated/vk_safe_struct.cpp
    ../layers/image_layout_map.cpp
    ../layers/subresource_adapter.cpp
    ../layers/generated/corechecks_optick_instrumentation.cpp
)

set_target_properties ( VkLayer_khronos_validation PROPERTIES
    CXX_VISIBILITY_PRESET hidden
)

target_include_directories ( VkLayer_khronos_validation
    PRIVATE
    Vulkan-Headers/include
    ../layers
    ../layers/generated
    SPIRV-Headers/include
    SPIRV-Tools/include
    robin-hood-hashing/src/include
)

target_compile_definitions ( VkLayer_khronos_validation
    PRIVATE
    -DVK_ENABLE_BETA_EXTENSIONS
    -DVK_USE_PLATFORM_ANDROID_KHR
    -DVK_PROTOTYPES
    -DUSE_ROBIN_HOOD_HASHING
)

set ( CMAKE_CXX_FLAGS
    -std=c++11
    -Wall
    -Werror
    -Wno-unused-function
    -Wno-unused-const-variable
    -frtti
)

target_link_libraries ( VkLayer_khronos_validation
    ${log-lib}
    android
    $ENV{VK_LAYER_DIR}/glslang/glslang/libglslang.a
    $ENV{VK_LAYER_DIR}/SPIRV-Tools/build/app/local/arm64-v8a/libSPIRV-Tools.a
    $ENV{VK_LAYER_DIR}/SPIRV-Tools/build/app/local/arm64-v8a/libSPIRV-Tools-opt.a
    layer_utils
)
