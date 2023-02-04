cmake_minimum_required ( VERSION 3.22.1 )
set ( CMAKE_CXX_STANDARD 20 )
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
    ../layers/vk_layer_logging.cpp
    ../layers/vk_layer_utils.cpp
    ../layers/generated/vk_format_utils.cpp
)

target_include_directories ( layer_utils
    PRIVATE
    Vulkan-Headers/include
    ../layers
    ../layers/generated
    robin-hood-hashing/src/include
    SPIRV-Tools/external/spirv-headers/include
)

target_compile_definitions ( layer_utils
    PRIVATE
    -DVK_ENABLE_BETA_EXTENSIONS
    -DVK_USE_PLATFORM_ANDROID_KHR
    -DVK_PROTOTYPES
    -DUSE_ROBIN_HOOD_HASHING
)

target_compile_options ( layer_utils PRIVATE
    -Wall
    -Werror
    -Wno-unused-function
    -Wno-unused-const-variable
    -fvisibility=hidden
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
    ../layers/state_tracker/state_tracker.cpp
    ../layers/core_checks/android_validation.cpp
    ../layers/core_checks/device_validation.cpp
    ../layers/state_tracker/device_memory_state.cpp
    ../layers/core_checks/device_memory_validation.cpp
    ../layers/core_checks/external_object_validation.cpp
    ../layers/state_tracker/base_node.cpp
    ../layers/state_tracker/buffer_state.cpp
    ../layers/state_tracker/cmd_buffer_state.cpp
    ../layers/core_checks/cmd_buffer_dynamic_validation.cpp
    ../layers/core_checks/cmd_buffer_validation.cpp
    ../layers/core_checks/copy_blit_resolve_validation.cpp
    ../layers/state_tracker/image_state.cpp
    ../layers/state_tracker/pipeline_state.cpp
    ../layers/state_tracker/pipeline_layout_state.cpp
    ../layers/state_tracker/pipeline_sub_state.cpp
    ../layers/core_checks/image_validation.cpp
    ../layers/core_checks/image_layout_validation.cpp
    ../layers/core_checks/pipeline_validation.cpp
    ../layers/state_tracker/queue_state.cpp
    ../layers/state_tracker/render_pass_state.cpp
    ../layers/core_checks/render_pass_validation.cpp
    ../layers/state_tracker/video_session_state.cpp
    ../layers/core_checks/video_validation.cpp
    ../layers/core_checks/drawdispatch_validation.cpp
    ../layers/state_tracker/descriptor_sets.cpp
    ../layers/core_checks/descriptor_validation.cpp
    ../layers/core_checks/buffer_validation.cpp
    ../layers/state_tracker/shader_module.cpp
    ../layers/state_tracker/shader_instruction.cpp
    ../layers/core_checks/shader_validation.cpp
    ../layers/core_checks/synchronization_validation.cpp
    ../layers/generated/spirv_validation_helper.cpp
    ../layers/generated/spirv_grammar_helper.cpp
    ../layers/generated/command_validation.cpp
    ../layers/gpu_validation/gpu_validation.cpp
    ../layers/gpu_validation/gpu_utils.cpp
    ../layers/gpu_validation/debug_printf.cpp
    ../layers/best_practices/best_practices_utils.cpp
    ../layers/generated/best_practices.cpp
    ../layers/sync/sync_utils.cpp
    ../layers/sync/sync_vuid_maps.cpp
    ../layers/core_error_location.cpp
    ../layers/generated/sync_validation_types.cpp
    ../layers/sync/sync_validation.cpp
    ../layers/convert_to_renderpass2.cpp
    ../layers/generated/layer_chassis_dispatch.cpp
    ../layers/generated/chassis.cpp
    ../layers/generated/valid_param_values.cpp
    ../layers/layer_options.cpp
    ../layers/core_checks/query_validation.cpp
    ../layers/core_checks/queue_validation.cpp
    ../layers/core_checks/ray_tracing_validation.cpp
    ../layers/core_checks/wsi_validation.cpp
    ../layers/xxhash.cpp
    ../layers/generated/parameter_validation.cpp
    ../layers/stateless/parameter_validation_utils.cpp
    ../layers/generated/object_tracker.cpp
    ../layers/object_tracker_utils.cpp
    ../layers/generated/thread_safety.cpp
    ../layers/generated/vk_safe_struct.cpp
    ../layers/state_tracker/image_layout_map.cpp
    ../layers/subresource_adapter.cpp
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
    -DXXH_NO_LONG_LONG
)

target_compile_options ( VkLayer_khronos_validation PRIVATE
    -Wall
    -Werror
    -Wno-unused-function
    -Wno-unused-const-variable
    -frtti
    -fvisibility=hidden
)

target_link_libraries ( VkLayer_khronos_validation
    ${log-lib}
    android
    $ENV{VK_LAYER_DIR}/glslang/glslang/libglslang.a
    $ENV{VK_LAYER_DIR}/SPIRV-Tools/build/app/local/arm64-v8a/libSPIRV-Tools.a
    $ENV{VK_LAYER_DIR}/SPIRV-Tools/build/app/local/arm64-v8a/libSPIRV-Tools-opt.a
    layer_utils
)
